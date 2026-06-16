#include "driver/epwm_driver.h"
#include "bsp/board_resources.h"

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

static bool g_epwm_enabled;

sscb_status_t sscb_epwm_driver_init_trip_chain(void)
{
    g_epwm_enabled = false;
#ifdef __TMS320C28XX__
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    EPWM_setTimeBasePeriod(EPWM3_BASE, (uint16_t)(SSCB_EPWM3_PERIOD_TICKS - 1u));
    EPWM_setTimeBaseCounter(EPWM3_BASE, 0u);
    EPWM_setTimeBaseCounterMode(EPWM3_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_setClockPrescaler(EPWM3_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    EPWM_setCounterCompareValue(EPWM3_BASE, EPWM_COUNTER_COMPARE_A,
                                (uint16_t)(SSCB_EPWM3_PERIOD_TICKS / 2u));
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    EPWM_setADCTriggerSource(EPWM3_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
    EPWM_setADCTriggerEventPrescale(EPWM3_BASE, EPWM_SOC_A, 1u);
    EPWM_enableADCTrigger(EPWM3_BASE, EPWM_SOC_A);

    XBAR_setEPWMMuxConfig(XBAR_TRIP4, XBAR_EPWM_MUX04_CMPSS3_CTRIPH);
    XBAR_enableEPWMMux(XBAR_TRIP4, XBAR_MUX04);

    EPWM_selectDigitalCompareTripInput(EPWM3_BASE, EPWM_DC_TRIP_TRIPIN4, EPWM_DC_TYPE_DCAH);
    EPWM_setTripZoneDigitalCompareEventCondition(EPWM3_BASE,
                                                 EPWM_TZ_DC_OUTPUT_A1,
                                                 EPWM_TZ_EVENT_DCXH_HIGH);
    EPWM_setDigitalCompareFilterInput(EPWM3_BASE, EPWM_DC_WINDOW_SOURCE_DCAEVT1);
    EPWM_setDigitalCompareBlankingEvent(EPWM3_BASE, EPWM_DC_WINDOW_START_TBCTR_ZERO);
    EPWM_setDigitalCompareWindowOffset(EPWM3_BASE, 0u);
    EPWM_setDigitalCompareWindowLength(EPWM3_BASE, SSCB_EPWM3_DC_BLANKING_TICKS);
    EPWM_enableDigitalCompareBlankingWindow(EPWM3_BASE);
    EPWM_setDigitalCompareEventSource(EPWM3_BASE,
                                      EPWM_DC_MODULE_A,
                                      EPWM_DC_EVENT_1,
                                      EPWM_DC_EVENT_SOURCE_FILT_SIGNAL);
    EPWM_setDigitalCompareEventSyncMode(EPWM3_BASE,
                                        EPWM_DC_MODULE_A,
                                        EPWM_DC_EVENT_1,
                                        EPWM_DC_EVENT_INPUT_NOT_SYNCED);

    EPWM_enableTripZoneSignals(EPWM3_BASE, EPWM_TZ_SIGNAL_DCAEVT1);
    EPWM_setTripZoneAction(EPWM3_BASE, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(EPWM3_BASE, EPWM_TZ_ACTION_EVENT_DCAEVT1, EPWM_TZ_ACTION_LOW);
    EPWM_clearTripZoneFlag(EPWM3_BASE,
                           EPWM_TZ_FLAG_OST | EPWM_TZ_FLAG_CBC | EPWM_TZ_FLAG_DCAEVT1);

    EPWM_setActionQualifierContSWForceAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
#endif
    return SSCB_OK;
}

void sscb_epwm_force_off(void)
{
    g_epwm_enabled = false;
#ifdef __TMS320C28XX__
    EPWM_setActionQualifierContSWForceAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
#endif
}

void sscb_epwm_allow_output(bool enable)
{
    g_epwm_enabled = enable;
#ifdef __TMS320C28XX__
    if (enable) {
        EPWM_setActionQualifierContSWForceAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_DISABLED);
    } else {
        EPWM_setActionQualifierContSWForceAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_SW_OUTPUT_LOW);
    }
#endif
}

bool sscb_epwm_output_enabled(void)
{
    return g_epwm_enabled;
}
