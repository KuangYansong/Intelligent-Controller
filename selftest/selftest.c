#include "selftest/selftest.h"

sscb_selftest_result_t sscb_selftest_run(const sscb_selftest_inputs_t *in)
{
    sscb_selftest_result_t result;
    result.flags = 0u;
    result.fault = SSCB_FAULT_NONE;

    if (in == 0) {
        result.flags = 0xFFFFu;
        result.fault = SSCB_FAULT_SAMPLE_ABNORMAL;
        return result;
    }
    if (!in->adc_range_ok) {
        result.flags |= 1u << 0;
        result.fault = SSCB_FAULT_SAMPLE_ABNORMAL;
    }
    if (!in->ntc_ok) {
        result.flags |= 1u << 1;
        result.fault = SSCB_FAULT_SAMPLE_ABNORMAL;
    }
    if (!in->cmpss_config_ok) {
        result.flags |= 1u << 2;
        result.fault = SSCB_FAULT_SAMPLE_ABNORMAL;
    }
    if (!in->driver_ready || !in->driver_fault_clear) {
        result.flags |= 1u << 3;
        result.fault = SSCB_FAULT_DRIVER;
    }
    return result;
}
