#ifndef SSCB_ADC_DRIVER_H
#define SSCB_ADC_DRIVER_H

#include <stdint.h>
#include "common/parameters.h"
#include "common/sscb_types.h"

typedef struct {
    uint16_t voltage_raw;
    uint16_t current_pga_raw;
    uint16_t current_raw_raw;
    uint16_t temp_raw;
} sscb_adc_raw_t;

sscb_status_t sscb_adc_driver_init(void);
sscb_adc_raw_t sscb_adc_driver_read_latest(void);
int16_t sscb_adc_ntc10k_beta_temp_dc(uint16_t adc_raw);
sscb_measurements_t sscb_adc_convert_measurements(const sscb_adc_raw_t *raw, const sscb_params_t *params);

#endif
