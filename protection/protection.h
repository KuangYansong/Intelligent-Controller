#ifndef SSCB_PROTECTION_H
#define SSCB_PROTECTION_H

#include <stdint.h>
#include "common/parameters.h"
#include "common/sscb_types.h"

#define SSCB_RMS_WINDOW_SAMPLES 250u

typedef struct {
    const sscb_params_t *params;
    int32_t samples_ma[SSCB_RMS_WINDOW_SAMPLES];
    uint16_t index;
    uint16_t count;
    uint64_t sum_sq_a2;
    uint32_t rms_a;
    uint32_t i2t_acc;
    uint16_t over_voltage_ms;
} sscb_protection_t;

void sscb_protection_init(sscb_protection_t *prot, const sscb_params_t *params);
void sscb_protection_sample_current_ma(sscb_protection_t *prot, int32_t current_ma);
sscb_fault_t sscb_protection_tick_1ms(sscb_protection_t *prot, const sscb_measurements_t *m);
uint32_t sscb_protection_rms_a(const sscb_protection_t *prot);

#endif
