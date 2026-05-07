#include "board.h"
#include "adc_driver.h"
#include "can_driver.h"
#include "cmpss_driver.h"
#include "epwm_driver.h"
#include "fram.h"
#include "system.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"

__interrupt static void board_epwm3_trip_isr(void)
{
    /* ePWM3 Trip 触发后，立即转给系统层记录短路/故障事件。 */
    System_OnShortTripInterrupt();
    EPWM_clearTripZoneFlag(EPWM3_BASE, EPWM_TZ_INTERRUPT | EPWM_TZ_FLAG_OST | EPWM_TZ_FLAG_DCAEVT1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP2);
}
#endif

SscbStatus Board_Init(void)
{
#ifdef SSCB_TARGET_C2000
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();
    Interrupt_register(INT_EPWM3_TZ, &board_epwm3_trip_isr);
    Interrupt_enable(INT_EPWM3_TZ);
#endif
    if (AdcDriver_Init() != SSCB_OK)
    {
        return SSCB_ERROR;
    }
    if (EpwmDriver_Init() != SSCB_OK)
    {
        return SSCB_ERROR;
    }
    if (Fram_Init() != SSCB_OK)
    {
        return SSCB_ERROR;
    }
#ifdef SSCB_TARGET_C2000
    Interrupt_enableMaster();
#endif
    return SSCB_OK;
}

void Board_Idle(void)
{
#ifdef SSCB_TARGET_C2000
    IDLE;
#endif
}

void Board_SoftwareReset(void)
{
#ifdef SSCB_TARGET_C2000
    SysCtl_resetDevice();
#endif
}
