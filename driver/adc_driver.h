#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "sscb_types.h"

/* 初始化 ADC 外设或测试模式下的采样缓存。 */
SscbStatus AdcDriver_Init(void);
/* 读取最近一次 ADC 原始采样值。 */
SscbAdcRaw AdcDriver_ReadLatest(void);
/* 把原始 ADC 计数换算成电压、电流、温度。 */
SscbMeasurements AdcDriver_Convert(const SscbAdcRaw *raw, const SscbParams *params);

#endif
