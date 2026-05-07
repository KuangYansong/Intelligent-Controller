#include "epwm_driver.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#endif

static bool s_trip;

SscbStatus EpwmDriver_Init(void)
{
    s_trip = false;
#ifdef SSCB_TARGET_C2000
    EPWM_setTimeBasePeriod(EPWM3_BASE, 2500u);
    EPWM_setTimeBaseCounter(EPWM3_BASE, 0u);
    EPWM_setTimeBaseCounterMode(EPWM3_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setCounterCompareValue(EPWM3_BASE, EPWM_COUNTER_COMPARE_A, 1250u);
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM3_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setTripZoneAction(EPWM3_BASE, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_FLAG_OST);
#endif
    return SSCB_OK;
}

void EpwmDriver_SetTrip(bool trip)
{
    s_trip = trip;
#ifdef SSCB_TARGET_C2000
    if (trip)
    {
        EPWM_forceTripZoneEvent(EPWM3_BASE, EPWM_TZ_FORCE_EVENT_OST);
    }
    else
    {
        EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_FLAG_OST);
    }
#else
    (void)s_trip;
#endif
}

