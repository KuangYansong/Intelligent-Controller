#ifndef SSCB_PARAMETERS_H
#define SSCB_PARAMETERS_H

#include <stdint.h>
#include "common/sscb_types.h"

typedef enum {
    SSCB_PARAM_NODE_ID = 0x01,
    SSCB_PARAM_SHORT_THRESHOLD_A = 0x02,
    SSCB_PARAM_OVER_CURRENT_IR_A = 0x03,
    SSCB_PARAM_I2T_TR_MS = 0x04,
    SSCB_PARAM_I2T_KCOOL_X10000 = 0x05,
    SSCB_PARAM_OVERLOAD_RECOVER_DELAY_MS = 0x06,
    SSCB_PARAM_OVER_VOLTAGE_TRIP_DV = 0x07,
    SSCB_PARAM_OVER_VOLTAGE_RETURN_DV = 0x08,
    SSCB_PARAM_OVER_VOLTAGE_TRIP_DELAY_MS = 0x09,
    SSCB_PARAM_OVER_VOLTAGE_RETURN_DELAY_MS = 0x0A,
    SSCB_PARAM_OVER_TEMP_WARN_DC = 0x0B,
    SSCB_PARAM_OVER_TEMP_RETURN_DC = 0x0C,
    SSCB_PARAM_RECOVER_MODE = 0x0D,
    SSCB_PARAM_CMPSS_DAC_REF_MV = 0x0E,
    SSCB_PARAM_VOLTAGE_GAIN_MV_PER_COUNT = 0x20,
    SSCB_PARAM_CURRENT_PGA_GAIN_MA_PER_COUNT = 0x22,
    SSCB_PARAM_CURRENT_RAW_GAIN_MA_PER_COUNT = 0x24
} sscb_param_id_t;

typedef struct {
    uint8_t node_id;
    uint16_t short_threshold_a;
    uint16_t over_current_ir_a;
    uint16_t i2t_tr_ms;
    uint16_t i2t_kcool_x10000;
    uint16_t overload_recover_delay_ms;
    uint16_t over_voltage_trip_dv;
    uint16_t over_voltage_return_dv;
    uint16_t over_voltage_trip_delay_ms;
    uint16_t over_voltage_return_delay_ms;
    int16_t over_temp_warn_dc;
    int16_t over_temp_return_dc;
    uint8_t recover_mode;
    uint16_t cmpss_dac_ref_mv;
    uint32_t voltage_gain_mv_per_count;
    int16_t voltage_offset_dv;
    uint32_t current_pga_gain_ma_per_count;
    int32_t current_pga_offset_ma;
    uint32_t current_raw_gain_ma_per_count;
    int32_t current_raw_offset_ma;
    uint16_t ntc_r_fixed_ohm;
    uint16_t ntc_r0_ohm;
    uint16_t ntc_b_k;
    uint16_t adc_vref_mv;
    uint16_t realtime_period_ms;
    uint16_t heartbeat_period_ms;
    uint16_t fault_record_capacity;
    uint16_t protocol_version;
    uint16_t parameter_version;
} sscb_params_t;

void sscb_params_load_defaults(sscb_params_t *params);
sscb_status_t sscb_params_set(sscb_params_t *params, uint8_t param_id, uint32_t value);
sscb_status_t sscb_params_get(const sscb_params_t *params, uint8_t param_id, uint32_t *value);
uint16_t sscb_short_threshold_to_dac_code(const sscb_params_t *params);

#endif
