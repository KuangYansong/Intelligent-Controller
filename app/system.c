#include "app/system.h"

static void append_fault(sscb_system_t *sys, sscb_fault_t fault, const sscb_measurements_t *m)
{
    sscb_fault_record_t rec;
    if (sys == 0 || m == 0 || fault == SSCB_FAULT_NONE) {
        return;
    }

    rec.sequence = (uint16_t)sys->fault_log.total_count;
    rec.fault = fault;
    rec.state = sys->state;
    rec.flags = 0u;
    rec.timestamp_ms = sys->time_ms;
    rec.voltage_dv = m->voltage_dv;
    rec.current_pga_da = m->current_pga_da;
    rec.current_raw_da = m->current_raw_da;
    rec.temperature_dc = m->temperature_dc;
    rec.i2t_value = sys->protection.i2t_acc;
    rec.adc_flags = m->adc_flags;
    rec.driver_flags = m->driver_flags;
    (void)sscb_fault_log_append(&sys->fault_log, &rec);
}

void sscb_system_init(sscb_system_t *sys)
{
    if (sys == 0) {
        return;
    }
    sscb_params_load_defaults(&sys->params);
    sscb_protection_init(&sys->protection, &sys->params);
    sscb_fault_log_init(&sys->fault_log);
    sys->state = SSCB_STATE_READY;
    sys->active_fault = SSCB_FAULT_NONE;
    sys->status_word = SSCB_STATUS_NORMAL;
    sys->time_ms = 0u;
    sys->recover_elapsed_ms = 0u;
    sys->trip_clear = true;
    sys->driver_ready = true;
    sys->fault_conditions_clear = true;
}

sscb_state_t sscb_system_state(const sscb_system_t *sys)
{
    return sys == 0 ? SSCB_STATE_INIT : sys->state;
}

sscb_status_t sscb_system_start(sscb_system_t *sys)
{
    if (sys == 0) {
        return SSCB_ERR_ARG;
    }
    if (sys->state != SSCB_STATE_READY) {
        return SSCB_ERR_STATE;
    }
    sys->state = SSCB_STATE_RUN;
    sys->status_word = SSCB_STATUS_NORMAL;
    return SSCB_OK;
}

void sscb_system_on_short_trip(sscb_system_t *sys, const sscb_measurements_t *m)
{
    if (sys == 0) {
        return;
    }
    sys->active_fault = SSCB_FAULT_SHORT;
    sys->state = SSCB_STATE_LOCKOUT;
    sys->status_word |= SSCB_STATUS_SHORT | SSCB_STATUS_LOCKOUT;
    if (m != 0) {
        append_fault(sys, SSCB_FAULT_SHORT, m);
    }
}

void sscb_system_tick_1ms(sscb_system_t *sys, const sscb_measurements_t *m)
{
    sscb_fault_t fault;
    if (sys == 0 || m == 0) {
        return;
    }

    sys->time_ms++;

    if (sys->state == SSCB_STATE_RUN || sys->state == SSCB_STATE_WARN) {
        fault = sscb_protection_tick_1ms(&sys->protection, m);
        if (fault == SSCB_FAULT_OVERTEMP_WARN) {
            sys->state = SSCB_STATE_WARN;
            sys->status_word |= SSCB_STATUS_OVERTEMP_WARN;
        } else if (fault != SSCB_FAULT_NONE) {
            sys->active_fault = fault;
            sys->state = SSCB_STATE_FAULT_ACTIVE;
            sys->status_word &= (uint16_t)~SSCB_STATUS_NORMAL;
            if (fault == SSCB_FAULT_OVERLOAD) {
                sys->status_word |= SSCB_STATUS_OVERLOAD;
            } else if (fault == SSCB_FAULT_OVERVOLTAGE) {
                sys->status_word |= SSCB_STATUS_OVERVOLTAGE;
            }
            append_fault(sys, fault, m);
        } else if (sys->state == SSCB_STATE_WARN && m->temperature_dc <= sys->params.over_temp_return_dc) {
            sys->state = SSCB_STATE_RUN;
            sys->status_word &= (uint16_t)~SSCB_STATUS_OVERTEMP_WARN;
            sys->status_word |= SSCB_STATUS_NORMAL;
        }
    } else if (sys->state == SSCB_STATE_FAULT_ACTIVE && sys->active_fault == SSCB_FAULT_OVERLOAD) {
        sys->state = SSCB_STATE_RECOVER_WAIT;
        sys->recover_elapsed_ms = 0u;
    } else if (sys->state == SSCB_STATE_RECOVER_WAIT) {
        if (sys->recover_elapsed_ms < UINT16_MAX) {
            sys->recover_elapsed_ms++;
        }
        if (sys->recover_elapsed_ms >= sys->params.overload_recover_delay_ms &&
            sys->fault_conditions_clear && sys->driver_ready && sys->trip_clear) {
            sys->state = SSCB_STATE_RECOVER_TRY;
        }
    } else if (sys->state == SSCB_STATE_RECOVER_TRY) {
        if (sys->fault_conditions_clear && sys->driver_ready && sys->trip_clear) {
            sys->active_fault = SSCB_FAULT_NONE;
            sys->state = SSCB_STATE_RUN;
            sys->status_word = SSCB_STATUS_NORMAL;
        } else {
            sys->state = SSCB_STATE_LOCKOUT;
            sys->status_word |= SSCB_STATUS_LOCKOUT;
        }
    }
}

void sscb_system_set_safety_inputs(sscb_system_t *sys, bool trip_clear,
                                   bool driver_ready, bool fault_conditions_clear)
{
    if (sys == 0) {
        return;
    }
    sys->trip_clear = trip_clear;
    sys->driver_ready = driver_ready;
    sys->fault_conditions_clear = fault_conditions_clear;
}

sscb_status_t sscb_system_manual_reset(sscb_system_t *sys)
{
    if (sys == 0) {
        return SSCB_ERR_ARG;
    }
    if (sys->state != SSCB_STATE_LOCKOUT) {
        return SSCB_ERR_STATE;
    }
    if (!sys->trip_clear || !sys->driver_ready || !sys->fault_conditions_clear) {
        return SSCB_ERR_STATE;
    }
    sys->active_fault = SSCB_FAULT_NONE;
    sys->state = SSCB_STATE_READY;
    sys->status_word = SSCB_STATUS_NORMAL;
    return SSCB_OK;
}
