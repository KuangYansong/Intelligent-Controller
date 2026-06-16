#include "driver/adc_driver.h"
#include "bsp/board_resources.h"
#include <math.h>

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

static sscb_adc_raw_t g_last_raw;

#define SSCB_NTC_ADC_FULL_SCALE 4096.0f
#define SSCB_NTC_R_FIXED_OHM 10000.0f
#define SSCB_NTC_R25_OHM 10000.0f
#define SSCB_NTC_BETA_K 3988.0f
#define SSCB_NTC_T25_K 298.15f
#define SSCB_KELVIN_TO_CELSIUS 273.15f

sscb_status_t sscb_adc_driver_init(void)
{
#ifdef __TMS320C28XX__
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCC_BASE, ADC_CLK_DIV_4_0);
    ADC_enableConverter(ADCA_BASE);
    ADC_enableConverter(ADCB_BASE);
    ADC_enableConverter(ADCC_BASE);
    DEVICE_DELAY_US(1000);

    PGA_setGain(PGA1_BASE, PGA_GAIN_6);
    PGA_enable(PGA1_BASE);

    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN2, 15u);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN3, 15u);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN1, 15u);
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM3_SOCA, ADC_CH_ADCIN4, 15u);
#endif
    return SSCB_OK;
}

sscb_adc_raw_t sscb_adc_driver_read_latest(void)
{
#ifdef __TMS320C28XX__
    g_last_raw.current_pga_raw = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    g_last_raw.current_raw_raw = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
    g_last_raw.temp_raw = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);
    g_last_raw.voltage_raw = ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER0);
#endif
    return g_last_raw;
}

int16_t sscb_adc_ntc10k_beta_temp_dc(uint16_t adc_raw)
{
    float r_ntc;
    float ln_ratio;
    float inv_t;
    float temp_c;
    int32_t temp_dc;

    if (adc_raw <= 0u) {
        return -500;
    }
    if (adc_raw >= 4095u) {
        return 2000;
    }

    r_ntc = SSCB_NTC_R_FIXED_OHM * ((float)adc_raw / (SSCB_NTC_ADC_FULL_SCALE - (float)adc_raw));
    ln_ratio = logf(r_ntc / SSCB_NTC_R25_OHM);
    inv_t = (1.0f / SSCB_NTC_T25_K) + ((1.0f / SSCB_NTC_BETA_K) * ln_ratio);
    temp_c = (1.0f / inv_t) - SSCB_KELVIN_TO_CELSIUS;
    temp_dc = (int32_t)((temp_c * 10.0f) + ((temp_c >= 0.0f) ? 0.5f : -0.5f));

    if (temp_dc > 32767) {
        return 32767;
    }
    if (temp_dc < -32768) {
        return -32768;
    }
    return (int16_t)temp_dc;
}

sscb_measurements_t sscb_adc_convert_measurements(const sscb_adc_raw_t *raw, const sscb_params_t *p)
{
    sscb_measurements_t m;
    int32_t current_ma;
    int32_t raw_current_ma;

    m.voltage_dv = 0u;
    m.current_pga_da = 0;
    m.current_raw_da = 0;
    m.temperature_dc = 0;
    m.adc_flags = 0u;
    m.driver_flags = 0u;

    if (raw == 0 || p == 0) {
        return m;
    }

    m.voltage_dv = (uint16_t)(((uint32_t)raw->voltage_raw * p->voltage_gain_mv_per_count) / 100u);
    if (p->voltage_offset_dv >= 0) {
        m.voltage_dv = (uint16_t)(m.voltage_dv + (uint16_t)p->voltage_offset_dv);
    }

    current_ma = (int32_t)raw->current_pga_raw * (int32_t)p->current_pga_gain_ma_per_count + p->current_pga_offset_ma;
    raw_current_ma = (int32_t)raw->current_raw_raw * (int32_t)p->current_raw_gain_ma_per_count + p->current_raw_offset_ma;
    m.current_pga_da = (int16_t)(current_ma / 100);
    m.current_raw_da = (int16_t)(raw_current_ma / 100);

    if (raw->current_pga_raw > 4000u) {
        m.adc_flags |= 1u;
    }
    if (raw->current_raw_raw > 0u) {
        m.adc_flags |= 2u;
    }

    if (raw->temp_raw < 10u || raw->temp_raw > 4085u) {
        m.adc_flags |= 4u;
    }

    m.temperature_dc = sscb_adc_ntc10k_beta_temp_dc(raw->temp_raw);

    return m;
}
