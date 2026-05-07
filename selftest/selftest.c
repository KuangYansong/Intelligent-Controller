#include "selftest.h"
#include "sscb_config.h"

SelfTestResult SelfTest_RunPowerOn(const SscbAdcRaw *raw)
{
    SelfTestResult result;
    result.adc_range_ok = (raw != 0) &&
                          (raw->voltage_raw <= SSCB_ADC_MAX_COUNTS) &&
                          (raw->current_raw <= SSCB_ADC_MAX_COUNTS) &&
                          (raw->temp_raw <= SSCB_ADC_MAX_COUNTS);
    result.sampling_ok = result.adc_range_ok;
    result.cmpss_config_ok = true;
    return result;
}

SelfTestResult SelfTest_RunPeriodic(const SscbMeasurements *m)
{
    SelfTestResult result;
    result.adc_range_ok = (m != 0) &&
                          (m->voltage_v >= 0.0f) &&
                          (m->current_a >= 0.0f) &&
                          (m->temperature_c > -50.0f) &&
                          (m->temperature_c < 150.0f);
    result.sampling_ok = result.adc_range_ok;
    result.cmpss_config_ok = true;
    return result;
}

bool SelfTest_Passed(const SelfTestResult *result)
{
    return (result != 0) && result->adc_range_ok && result->sampling_ok && result->cmpss_config_ok;
}

