#include "adc_driver.h"
#include "sscb_config.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#endif

static SscbAdcRaw s_latest_raw;

SscbStatus AdcDriver_Init(void)
{
#ifdef SSCB_TARGET_C2000
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCB_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_CONV);
    ADC_enableConverter(ADCB_BASE);
    DEVICE_DELAY_US(1000);

    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 14);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN1, 14);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN3, 14);
    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER2);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER1);
#endif
    s_latest_raw.voltage_raw = 0u;
    s_latest_raw.current_raw = 0u;
    s_latest_raw.temp_raw = 0u;
    return SSCB_OK;
}

SscbAdcRaw AdcDriver_ReadLatest(void)
{
#ifdef SSCB_TARGET_C2000
    s_latest_raw.current_raw = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
    s_latest_raw.temp_raw = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    s_latest_raw.voltage_raw = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2);
#endif
    return s_latest_raw;
}

SscbMeasurements AdcDriver_Convert(const SscbAdcRaw *raw, const SscbParams *params)
{
    SscbMeasurements m;
    float voltage = 0.0f;
    float current = 0.0f;
    float temp = 0.0f;

    if (raw != 0)
    {
        voltage = ((float)raw->voltage_raw * (float)SSCB_ADC_REF_MV / (float)SSCB_ADC_MAX_COUNTS) / 1000.0f;
        current = ((float)raw->current_raw * (float)SSCB_ADC_REF_MV / (float)SSCB_ADC_MAX_COUNTS) / 1000.0f;
        temp = ((float)raw->temp_raw * (float)SSCB_ADC_REF_MV / (float)SSCB_ADC_MAX_COUNTS) / 1000.0f;
    }

    m.voltage_v = voltage * ((params != 0) ? params->voltage_k : 1.0f);
    m.current_a = current * ((params != 0) ? params->current_k : 1.0f);
    m.current_rms_a = m.current_a;
    m.temperature_c = temp * 100.0f * ((params != 0) ? params->temp_k : 1.0f);
    return m;
}

