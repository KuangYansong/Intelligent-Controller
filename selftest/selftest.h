#ifndef SSCB_SELFTEST_H
#define SSCB_SELFTEST_H

#include <stdbool.h>
#include <stdint.h>
#include "common/sscb_types.h"

typedef struct {
    bool adc_range_ok;
    bool ntc_ok;
    bool cmpss_config_ok;
    bool driver_ready;
    bool driver_fault_clear;
} sscb_selftest_inputs_t;

typedef struct {
    uint16_t flags;
    sscb_fault_t fault;
} sscb_selftest_result_t;

sscb_selftest_result_t sscb_selftest_run(const sscb_selftest_inputs_t *inputs);

#endif
