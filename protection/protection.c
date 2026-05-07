#include "protection.h"
#include "sscb_config.h"

static void set_trip(ProtectionService *svc, bool enable)
{
    /* 保护层不直接操作 PWM/硬件脚，而是通过回调控制跳闸输出。 */
    if (svc->trip_output != 0)
    {
        svc->trip_output(enable);
    }
}

static SscbFaultCode raise_fault(ProtectionService *svc, SscbFaultCode fault, uint32_t now_ms)
{
    /* 已经进入锁定态时，不再重复接受新的故障触发。 */
    if ((svc->state == SSCB_STATE_LOCKOUT) || (fault == SSCB_FAULT_NONE))
    {
        return SSCB_FAULT_NONE;
    }

    /* 记录故障、更新时间戳、拉起跳闸输出，并切到故障活动态。 */
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

    /* 初始化时默认处于正常状态，所有累计量清零。 */
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
    /* 每来一个电流采样，就更新一次 RMS 滑动窗口。 */
    return Rms_AddSample(&svc->rms, current_a);
}

SscbFaultCode Protection_Evaluate10ms(ProtectionService *svc, const SscbParams *params, SscbMeasurements *m, uint32_t now_ms)
{
    /* 只有在正常态下才做常规保护判断；故障态交给恢复逻辑处理。 */
    if ((svc == 0) || (params == 0) || (m == 0) || (svc->state != SSCB_STATE_NORMAL))
    {
        return SSCB_FAULT_NONE;
    }

    /* 重新取出当前窗口内的 RMS 电流，作为过载判断依据。 */
    m->current_rms_a = Rms_Get(&svc->rms);

    /* 先过滤明显不合理的采样值，防止坏数据误导后续保护逻辑。 */
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
        /* 过流不是瞬间判故障，而是通过 I2T 累积体现“过载持续时间”。 */
        return raise_fault(svc, SSCB_FAULT_OVERLOAD, now_ms);
    }

    return SSCB_FAULT_NONE;
}

SscbFaultCode Protection_OnShortTrip(ProtectionService *svc, const SscbMeasurements *m, uint32_t now_ms)
{
    (void)m;
    /* 短路通常由硬件快速比较器触发，因此这里直接上报短路故障。 */
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

    /* 故障触发后，先等待一个冷却/观察时间，再进入恢复等待态。 */
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
        /* 只有电压、温度、电流都回到安全范围内，才允许尝试恢复。 */
        bool safe = (m->voltage_v <= params->overvoltage_threshold_v) &&
                    (m->temperature_c <= params->overtemp_threshold_c) &&
                    (m->current_rms_a <= params->overcurrent_threshold_a);

        if (!safe)
        {
            /* 已经恢复过一次却仍不安全，直接进入锁定，等待人工干预。 */
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

        /* 执行一次自动恢复：撤销跳闸，清空 I2T 累积，回到正常态。 */
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

    /* 仅在电流几乎为零时允许清故障，避免带载强行复位。 */
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
    /* 只要还在故障态、恢复等待态或锁定态，都认为处于跳闸状态。 */
    return (svc != 0) && ((svc->state == SSCB_STATE_FAULT_ACTIVE) || (svc->state == SSCB_STATE_RECOVER_WAIT) || (svc->state == SSCB_STATE_LOCKOUT));
}
