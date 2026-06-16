#include "protection/protection.h"

static uint32_t isqrt_u64(uint64_t x)
{
    uint64_t op = x;
    uint64_t res = 0;
    uint64_t one = (uint64_t)1 << 62;

    while (one > op) {
        one >>= 2;
    }
    while (one != 0u) {
        if (op >= res + one) {
            op -= res + one;
            res = (res >> 1) + one;
        } else {
            res >>= 1;
        }
        one >>= 2;
    }
    return (uint32_t)res;
}

void sscb_protection_init(sscb_protection_t *prot, const sscb_params_t *params)
{
    uint16_t i;
    if (prot == 0) {
        return;
    }
    prot->params = params;
    prot->index = 0u;
    prot->count = 0u;
    prot->sum_sq_a2 = 0u;
    prot->rms_a = 0u;
    prot->i2t_acc = 0u;
    prot->over_voltage_ms = 0u;
    for (i = 0u; i < SSCB_RMS_WINDOW_SAMPLES; i++) {
        prot->samples_ma[i] = 0;
    }
}

void sscb_protection_sample_current_ma(sscb_protection_t *prot, int32_t current_ma)
{
    int32_t old_ma;
    uint32_t old_a;
    uint32_t new_a;

    if (prot == 0) {
        return;
    }

    old_ma = prot->samples_ma[prot->index];
    old_a = (uint32_t)((old_ma < 0 ? -old_ma : old_ma) / 1000);
    new_a = (uint32_t)((current_ma < 0 ? -current_ma : current_ma) / 1000);

    if (prot->count == SSCB_RMS_WINDOW_SAMPLES) {
        prot->sum_sq_a2 -= (uint64_t)old_a * (uint64_t)old_a;
    } else {
        prot->count++;
    }

    prot->samples_ma[prot->index] = current_ma;
    prot->sum_sq_a2 += (uint64_t)new_a * (uint64_t)new_a;
    prot->index = (uint16_t)((prot->index + 1u) % SSCB_RMS_WINDOW_SAMPLES);

    if (prot->count != 0u) {
        prot->rms_a = isqrt_u64(prot->sum_sq_a2 / prot->count);
    }
}

uint32_t sscb_protection_rms_a(const sscb_protection_t *prot)
{
    return prot == 0 ? 0u : prot->rms_a;
}

sscb_fault_t sscb_protection_tick_1ms(sscb_protection_t *prot, const sscb_measurements_t *m)
{
    uint32_t ir;
    uint32_t rms;
    uint32_t trip_acc;
    uint32_t excess;
    const sscb_params_t *p;

    if (prot == 0 || prot->params == 0 || m == 0) {
        return SSCB_FAULT_NONE;
    }

    p = prot->params;
    rms = prot->rms_a;
    ir = p->over_current_ir_a;
    trip_acc = ((225u * ir * ir) / 100u - ir * ir) * p->i2t_tr_ms;

    if (rms > ir) {
        excess = rms * rms - ir * ir;
        if (UINT32_MAX - prot->i2t_acc < excess) {
            prot->i2t_acc = UINT32_MAX;
        } else {
            prot->i2t_acc += excess;
        }
    } else {
        prot->i2t_acc = (uint32_t)(((uint64_t)prot->i2t_acc * p->i2t_kcool_x10000) / 10000u);
    }

    if (prot->i2t_acc >= trip_acc) {
        return SSCB_FAULT_OVERLOAD;
    }

    if (m->voltage_dv >= p->over_voltage_trip_dv) {
        if (prot->over_voltage_ms < UINT16_MAX) {
            prot->over_voltage_ms++;
        }
        if (prot->over_voltage_ms >= p->over_voltage_trip_delay_ms) {
            return SSCB_FAULT_OVERVOLTAGE;
        }
    } else if (m->voltage_dv <= p->over_voltage_return_dv) {
        prot->over_voltage_ms = 0u;
    }

    if (m->temperature_dc >= p->over_temp_warn_dc) {
        return SSCB_FAULT_OVERTEMP_WARN;
    }

    return SSCB_FAULT_NONE;
}
