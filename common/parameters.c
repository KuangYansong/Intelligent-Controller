#include "common/parameters.h"

void sscb_params_load_defaults(sscb_params_t *p)
{
    if (p == 0) {
        return;
    }
    p->node_id = 1u;
    p->short_threshold_a = 1000u;
    p->over_current_ir_a = 250u;
    p->i2t_tr_ms = 5000u;
    p->i2t_kcool_x10000 = 9879u;
    p->overload_recover_delay_ms = 15000u;
    p->over_voltage_trip_dv = 9600u;
    p->over_voltage_return_dv = 8800u;
    p->over_voltage_trip_delay_ms = 500u;
    p->over_voltage_return_delay_ms = 500u;
    p->over_temp_warn_dc = 700;
    p->over_temp_return_dc = 650;
    p->recover_mode = 0u;
    p->cmpss_dac_ref_mv = 3000u;
    p->voltage_gain_mv_per_count = 412u;
    p->voltage_offset_dv = 0;
    p->current_pga_gain_ma_per_count = 224u;
    p->current_pga_offset_ma = 0;
    p->current_raw_gain_ma_per_count = 1343u;
    p->current_raw_offset_ma = 0;
    p->ntc_r_fixed_ohm = 10000u;
    p->ntc_r0_ohm = 10000u;
    p->ntc_b_k = 3988u;
    p->adc_vref_mv = 3000u;
    p->realtime_period_ms = 20u;
    p->heartbeat_period_ms = 100u;
    p->fault_record_capacity = 50u;
    p->protocol_version = 0x0101u;
    p->parameter_version = 0x0100u;
}

static int in_range_u32(uint32_t value, uint32_t min, uint32_t max)
{
    return value >= min && value <= max;
}

sscb_status_t sscb_params_set(sscb_params_t *p, uint8_t id, uint32_t value)
{
    if (p == 0) {
        return SSCB_ERR_ARG;
    }

    switch (id) {
    case SSCB_PARAM_NODE_ID:
        if (!in_range_u32(value, 1u, 127u)) return SSCB_ERR_RANGE;
        p->node_id = (uint8_t)value;
        return SSCB_OK;
    case SSCB_PARAM_SHORT_THRESHOLD_A:
        if (!in_range_u32(value, 500u, 2000u)) return SSCB_ERR_RANGE;
        p->short_threshold_a = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_CURRENT_IR_A:
        if (!in_range_u32(value, 100u, 500u)) return SSCB_ERR_RANGE;
        p->over_current_ir_a = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_I2T_TR_MS:
        if (!in_range_u32(value, 100u, 60000u)) return SSCB_ERR_RANGE;
        p->i2t_tr_ms = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_I2T_KCOOL_X10000:
        if (!in_range_u32(value, 0u, 10000u)) return SSCB_ERR_RANGE;
        p->i2t_kcool_x10000 = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVERLOAD_RECOVER_DELAY_MS:
        if (!in_range_u32(value, 0u, 60000u)) return SSCB_ERR_RANGE;
        p->overload_recover_delay_ms = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_TRIP_DV:
        if (!in_range_u32(value, 8000u, 11000u)) return SSCB_ERR_RANGE;
        p->over_voltage_trip_dv = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_RETURN_DV:
        if (!in_range_u32(value, 7500u, 10500u)) return SSCB_ERR_RANGE;
        p->over_voltage_return_dv = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_TRIP_DELAY_MS:
        if (!in_range_u32(value, 0u, 5000u)) return SSCB_ERR_RANGE;
        p->over_voltage_trip_delay_ms = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_RETURN_DELAY_MS:
        if (!in_range_u32(value, 0u, 5000u)) return SSCB_ERR_RANGE;
        p->over_voltage_return_delay_ms = (uint16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_TEMP_WARN_DC:
        if (!in_range_u32(value, 0u, 1550u)) return SSCB_ERR_RANGE;
        p->over_temp_warn_dc = (int16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_OVER_TEMP_RETURN_DC:
        if (!in_range_u32(value, 0u, 1550u)) return SSCB_ERR_RANGE;
        p->over_temp_return_dc = (int16_t)value;
        return SSCB_OK;
    case SSCB_PARAM_RECOVER_MODE:
        if (!in_range_u32(value, 0u, 255u)) return SSCB_ERR_RANGE;
        p->recover_mode = (uint8_t)value;
        return SSCB_OK;
    case SSCB_PARAM_CMPSS_DAC_REF_MV:
        if (!in_range_u32(value, 2500u, 3000u)) return SSCB_ERR_RANGE;
        p->cmpss_dac_ref_mv = (uint16_t)value;
        return SSCB_OK;
    default:
        return SSCB_ERR_NOT_FOUND;
    }
}

sscb_status_t sscb_params_get(const sscb_params_t *p, uint8_t id, uint32_t *value)
{
    if (p == 0 || value == 0) {
        return SSCB_ERR_ARG;
    }

    switch (id) {
    case SSCB_PARAM_NODE_ID: *value = p->node_id; return SSCB_OK;
    case SSCB_PARAM_SHORT_THRESHOLD_A: *value = p->short_threshold_a; return SSCB_OK;
    case SSCB_PARAM_OVER_CURRENT_IR_A: *value = p->over_current_ir_a; return SSCB_OK;
    case SSCB_PARAM_I2T_TR_MS: *value = p->i2t_tr_ms; return SSCB_OK;
    case SSCB_PARAM_I2T_KCOOL_X10000: *value = p->i2t_kcool_x10000; return SSCB_OK;
    case SSCB_PARAM_OVERLOAD_RECOVER_DELAY_MS: *value = p->overload_recover_delay_ms; return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_TRIP_DV: *value = p->over_voltage_trip_dv; return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_RETURN_DV: *value = p->over_voltage_return_dv; return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_TRIP_DELAY_MS: *value = p->over_voltage_trip_delay_ms; return SSCB_OK;
    case SSCB_PARAM_OVER_VOLTAGE_RETURN_DELAY_MS: *value = p->over_voltage_return_delay_ms; return SSCB_OK;
    case SSCB_PARAM_OVER_TEMP_WARN_DC: *value = (uint32_t)p->over_temp_warn_dc; return SSCB_OK;
    case SSCB_PARAM_OVER_TEMP_RETURN_DC: *value = (uint32_t)p->over_temp_return_dc; return SSCB_OK;
    case SSCB_PARAM_RECOVER_MODE: *value = p->recover_mode; return SSCB_OK;
    case SSCB_PARAM_CMPSS_DAC_REF_MV: *value = p->cmpss_dac_ref_mv; return SSCB_OK;
    case SSCB_PARAM_VOLTAGE_GAIN_MV_PER_COUNT: *value = p->voltage_gain_mv_per_count; return SSCB_OK;
    case SSCB_PARAM_CURRENT_PGA_GAIN_MA_PER_COUNT: *value = p->current_pga_gain_ma_per_count; return SSCB_OK;
    case SSCB_PARAM_CURRENT_RAW_GAIN_MA_PER_COUNT: *value = p->current_raw_gain_ma_per_count; return SSCB_OK;
    default: return SSCB_ERR_NOT_FOUND;
    }
}

uint16_t sscb_short_threshold_to_dac_code(const sscb_params_t *p)
{
    uint32_t dac;
    uint32_t ref;
    if (p == 0 || p->cmpss_dac_ref_mv == 0u) {
        return 0u;
    }

    ref = p->cmpss_dac_ref_mv;
    dac = ((uint32_t)p->short_threshold_a * 6u * 4095u + (ref * 10u / 2u)) / (ref * 10u);
    if (dac > 4095u) {
        dac = 4095u;
    }
    return (uint16_t)dac;
}
