#include "epwm_driver.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#include "pin_map.h"
#endif

static bool s_trip;

SscbStatus EpwmDriver_Init(void)
{
    /* 先记为未跳闸。 */
    s_trip = false;
#ifdef SSCB_TARGET_C2000
    /* GPIO4 输出 ePWM3A，GPIO3 采样外部故障输入 FLT_N。 */
    GPIO_setPinConfig(GPIO_4_EPWM3A);
    GPIO_setDirectionMode(4u, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_3_GPIO3);
    GPIO_setDirectionMode(3u, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(3u, GPIO_QUAL_ASYNC);

    /* FLT_N 从 GPIO3 进入 INPUT XBAR3，再送入 ePWM XBAR 的 TRIP5。 */
    XBAR_setInputPin(INPUTXBAR_BASE, XBAR_INPUT3, 3u);
    XBAR_setEPWMMuxConfig(XBAR_TRIP5, XBAR_EPWM_MUX05_INPUTXBAR3);
    XBAR_enableEPWMMux(XBAR_TRIP5, XBAR_MUX05);

    /* 配置 ePWM3 输出波形和 Trip Zone 行为。 */
    EPWM_setTimeBasePeriod(EPWM3_BASE, 2500u);
    EPWM_setTimeBaseCounter(EPWM3_BASE, 0u);
    EPWM_setTimeBaseCounterMode(EPWM3_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setCounterCompareValue(EPWM3_BASE, EPWM_COUNTER_COMPARE_A, 1250u);
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setADCTriggerSource(EPWM3_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
    EPWM_setADCTriggerEventPrescale(EPWM3_BASE, EPWM_SOC_A, 1u);
    EPWM_enableADCTrigger(EPWM3_BASE, EPWM_SOC_A);

    /* 组合 CMPSS3(TRIP4) 和外部故障 FLT_N(TRIP5)，任一拉高都触发 DCAEVT1。 */
    EPWM_selectDigitalCompareTripInput(EPWM3_BASE, EPWM_DC_TRIP_COMBINATION, EPWM_DC_TYPE_DCAH);
    EPWM_enableDigitalCompareTripCombinationInput(EPWM3_BASE, EPWM_DC_COMBINATIONAL_TRIPIN4, EPWM_DC_TYPE_DCAH);
    EPWM_enableDigitalCompareTripCombinationInput(EPWM3_BASE, EPWM_DC_COMBINATIONAL_TRIPIN5, EPWM_DC_TYPE_DCAH);
    EPWM_setDigitalCompareEventSource(EPWM3_BASE, EPWM_DC_MODULE_A, EPWM_DC_EVENT_1, EPWM_DC_EVENT_SOURCE_ORIG_SIGNAL);
    EPWM_setDigitalCompareEventSyncMode(EPWM3_BASE, EPWM_DC_MODULE_A, EPWM_DC_EVENT_1, EPWM_DC_EVENT_INPUT_NOT_SYNCED);
    EPWM_setTripZoneDigitalCompareEventCondition(EPWM3_BASE, EPWM_TZ_DC_OUTPUT_A1, EPWM_TZ_EVENT_DCXH_HIGH);

    EPWM_setTripZoneAction(EPWM3_BASE, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    EPWM_enableTripZoneSignals(EPWM3_BASE, EPWM_TZ_SIGNAL_DCAEVT1);
    EPWM_enableTripZoneInterrupt(EPWM3_BASE, EPWM_TZ_INTERRUPT_DCAEVT1);
    EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST | EPWM_TZ_FLAG_DCAEVT1);
#endif
    return SSCB_OK;
}

void EpwmDriver_SetTrip(bool trip)
{
    /* 把当前跳闸状态缓存下来，便于主机模式下观察逻辑结果。 */
    s_trip = trip;
#ifdef SSCB_TARGET_C2000
    if (trip)
    {
        /* 强制一次性跳闸事件，立即关断输出。 */
        EPWM_forceTripZoneEvent(EPWM3_BASE, EPWM_TZ_FORCE_EVENT_OST);
    }
    else
    {
        /* 清除跳闸标志，允许 PWM 恢复输出。 */
        EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST | EPWM_TZ_FLAG_DCAEVT1);
    }
#else
    (void)s_trip;
#endif
}
