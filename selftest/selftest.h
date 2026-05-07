#ifndef SELFTEST_H
#define SELFTEST_H

#include <stdbool.h>
#include "sscb_types.h"

typedef struct
{
    bool adc_range_ok;
    bool sampling_ok;
    bool cmpss_config_ok;
} SelfTestResult;

SelfTestResult SelfTest_RunPowerOn(const SscbAdcRaw *raw);
SelfTestResult SelfTest_RunPeriodic(const SscbMeasurements *m);
bool SelfTest_Passed(const SelfTestResult *result);

#endif

