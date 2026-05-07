#include "protection.h"
#include "sscb_config.h"

static void set_trip(ProtectionService *svc, bool enable)
{
    if (svc->trip_output != 0)
    {
        svc->trip_output(enable);
    }
}

static SscbFaultCode raise_fault(ProtectionService *svc, SscbFaultCode fault, uint32_t now_ms)
{
    if ((svc->state == SSCB_STATE_LOCKOUT) || (fault == SSCB_FAULT_NONE))
    {
        return SSCB_FAULT_NONE;
    }

    svc->active_fault = fault;
    svc->fault_bits |= (1UL << fault);
    svc->fault_started_ms = now_ms;
    svc->recover_started_ms = now_ms;
    svc->state = SSCB_STATE_FAULT_ACTIVE;
    set_trip(svc, true);
    return fault;
}

void Protection_Init(ProtectionService *svc, ProtectionTripOutput trip_output)
{
    if (svc == 0)
    {
        return;
    }

    svc->state = SSCB_STATE_NORMAL;
    svc->active_fault = SSCB_FAULT_NONE;
    svc->fault_bits = 0u;
    svc->fault_started_ms = 0u;
    svc->recover_started_ms = 0u;
    svc->auto_recover_used = false;
    svc->selftest_failed = false;
    svc->trip_output = trip_output;
    Rms_Init(&svc->rms);
    I2t_Init(&svc->i2t);
    set_trip(svc, false);
}

float Protection_AddCurrentSample(ProtectionService *svc, float current_a)
{
    if (svc == 0)
    {
        return 0.0f;
    }
    return Rms_AddSample(&svc->rms, current_a);
}

SscbFaultCode Protection_Evaluate10ms(ProtectionService *svc, const SscbParams *params, SscbMeasurements *m, uint32_t now_ms)
{
    if ((svc == 0) || (params == 0) || (m == 0) || (svc->state != SSCB_STATE_NORMAL))
    {
        return SSCB_FAULT_NONE;
    }

    m->current_rms_a = Rms_Get(&svc->rms);

    if ((m->voltage_v < 0.0f) || (m->current_a < 0.0f) || (m->temperature_c < -50.0f) || (m->temperature_c > 150.0f))
    {
        return raise_fault(svc, SSCB_FAULT_SAMPLE, now_ms);
    }
    if (m->voltage_v > params->overvoltage_threshold_v)
    {
        return raise_fault(svc, SSCB_FAULT_OVERVOLTAGE, now_ms);
    }
    if (m->temperature_c > params->overtemp_threshold_c)
    {
        return raise_fault(svc, SSCB_FAULT_OVERTEMP, now_ms);
    }
    if (I2t_Update(&svc->i2t, m->current_rms_a, params->overcurrent_threshold_a, params->i2t_threshold, 0.010f))
    {
        return raise_fault(svc, SSCB_FAULT_OVERLOAD, now_ms);
    }

    return SSCB_FAULT_NONE;
}

SscbFaultCode Protection_OnShortTrip(ProtectionService *svc, const SscbMeasurements *m, uint32_t now_ms)
{
    (void)m;
    if (svc == 0)
    {
        return SSCB_FAULT_NONE;
    }
    return raise_fault(svc, SSCB_FAULT_SHORT, now_ms);
}

bool Protection_UpdateRecovery(ProtectionService *svc, const SscbParams *params, const SscbMeasurements *m, uint32_t now_ms)
{
    if ((svc == 0) || (params == 0) || (m == 0))
    {
        return false;
    }

    if (svc->state == SSCB_STATE_FAULT_ACTIVE)
    {
        uint32_t delay_ms = (svc->active_fault == SSCB_FAULT_SHORT) ? SSCB_SHORT_RECOVER_DELAY_MS : SSCB_OVERLOAD_RECOVER_DELAY_MS;
        if ((uint32_t)(now_ms - svc->fault_started_ms) >= delay_ms)
        {
            svc->state = SSCB_STATE_RECOVER_WAIT;
            svc->recover_started_ms = now_ms;
        }
    }

    if (svc->state == SSCB_STATE_RECOVER_WAIT)
    {
        bool safe = (m->voltage_v <= params->overvoltage_threshold_v) &&
                    (m->temperature_c <= params->overtemp_threshold_c) &&
                    (m->current_rms_a <= params->overcurrent_threshold_a);

        if (!safe)
        {
            if (svc->auto_recover_used)
            {
                svc->state = SSCB_STATE_LOCKOUT;
            }
            return false;
        }

        if (svc->auto_recover_used)
        {
            svc->state = SSCB_STATE_LOCKOUT;
            return false;
        }

        svc->state = SSCB_STATE_RECOVER_TRY;
        svc->auto_recover_used = true;
        set_trip(svc, false);
        I2t_Reset(&svc->i2t);
        svc->state = SSCB_STATE_NORMAL;
        svc->active_fault = SSCB_FAULT_NONE;
        return true;
    }

    return false;
}

void Protection_ClearFaults(ProtectionService *svc, const SscbMeasurements *m)
{
    if ((svc == 0) || (m == 0))
    {
        return;
    }

    if ((m->current_rms_a <= 0.1f) && (m->current_a <= 0.1f))
    {
        svc->state = SSCB_STATE_NORMAL;
        svc->active_fault = SSCB_FAULT_NONE;
        svc->fault_bits = 0u;
        svc->auto_recover_used = false;
        I2t_Reset(&svc->i2t);
        set_trip(svc, false);
    }
}

bool Protection_IsTripped(const ProtectionService *svc)
{
    return (svc != 0) && ((svc->state == SSCB_STATE_FAULT_ACTIVE) || (svc->state == SSCB_STATE_RECOVER_WAIT) || (svc->state == SSCB_STATE_LOCKOUT));
}

