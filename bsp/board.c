#include "board.h"
#include "adc_driver.h"
#include "can_driver.h"
#include "cmpss_driver.h"
#include "epwm_driver.h"
#include "fram.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#endif

SscbStatus Board_Init(void)
{
#ifdef SSCB_TARGET_C2000
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();
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

