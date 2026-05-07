#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "sscb_types.h"

SscbStatus AdcDriver_Init(void);
SscbAdcRaw AdcDriver_ReadLatest(void);
SscbMeasurements AdcDriver_Convert(const SscbAdcRaw *raw, const SscbParams *params);

#endif

