#include "cmpss_driver.h"
#include "sscb_config.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#endif

static uint16_t threshold_to_dac(float threshold_a)
{
    /* 这里用默认短路阈值做归一化，把安培值映射到 DAC 码值。 */
    float ratio = threshold_a / SSCB_DEFAULT_SHORT_A;
    if (ratio < 0.0f)
    {
        ratio = 0.0f;
    }
    if (ratio > 1.0f)
    {
        ratio = 1.0f;
    }
    return (uint16_t)(ratio * 4095.0f);
}

SscbStatus CmpssDriver_Init(float short_threshold_a)
{
#ifdef SSCB_TARGET_C2000
    /* 配置 CMPSS3 的高比较器和内部 DAC，用于短路快速检测。 */
    CMPSS_enableModule(CMPSS3_BASE);
    CMPSS_configHighComparator(CMPSS3_BASE, CMPSS_INSRC_DAC);
    CMPSS_configDAC(CMPSS3_BASE, CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);
    CMPSS_setDACValueHigh(CMPSS3_BASE, threshold_to_dac(short_threshold_a));
    CMPSS_configOutputsHigh(CMPSS3_BASE, CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_ASYNC_COMP);
#else
    (void)short_threshold_a;
#endif
    return SSCB_OK;
}

void CmpssDriver_SetThreshold(float short_threshold_a)
{
#ifdef SSCB_TARGET_C2000
    /* 参数更新后，把新的短路门限同步到比较器 DAC。 */
    CMPSS_setDACValueHigh(CMPSS3_BASE, threshold_to_dac(short_threshold_a));
#else
    (void)short_threshold_a;
#endif
}
