#include "driver/cmpss_driver.h"

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

static uint16_t g_cmpss_dac_code;

sscb_status_t sscb_cmpss_driver_init(uint16_t dac_code)
{
    sscb_status_t rc;
#ifdef __TMS320C28XX__
    CMPSS_enableModule(CMPSS3_BASE);
    CMPSS_configHighComparator(CMPSS3_BASE, CMPSS_INSRC_DAC);
    CMPSS_configDAC(CMPSS3_BASE, CMPSS_DACVAL_SYSCLK | CMPSS_DACREF_VDDA | CMPSS_DACSRC_SHDW);
    CMPSS_configOutputsHigh(CMPSS3_BASE, CMPSS_TRIPOUT_ASYNC_COMP | CMPSS_TRIP_ASYNC_COMP);
#endif
    rc = sscb_cmpss_driver_set_dac(dac_code);
#ifdef __TMS320C28XX__
    CMPSS_initFilterHigh(CMPSS3_BASE);
#endif
    return rc;
}

sscb_status_t sscb_cmpss_driver_set_dac(uint16_t dac_code)
{
    if (dac_code > 4095u) {
        return SSCB_ERR_RANGE;
    }
    g_cmpss_dac_code = dac_code;
#ifdef __TMS320C28XX__
    CMPSS_setDACValueHigh(CMPSS3_BASE, dac_code);
#endif
    return SSCB_OK;
}

uint16_t sscb_cmpss_driver_last_dac(void)
{
    return g_cmpss_dac_code;
}
