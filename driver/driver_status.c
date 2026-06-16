#include "driver/driver_status.h"
#include "bsp/board_resources.h"

#ifdef __TMS320C28XX__
#include "driverlib.h"
#endif

sscb_status_t sscb_driver_status_init(void)
{
    return SSCB_OK;
}

sscb_driver_status_t sscb_driver_status_read(void)
{
    sscb_driver_status_t status;
#ifdef __TMS320C28XX__
    uint32_t flt_n = GPIO_readPin(SSCB_GPIO_FLT_N);
    uint32_t rdyc = GPIO_readPin(SSCB_GPIO_RDYC);
    status.fault_clear = flt_n != 0u;
    status.ready = rdyc != 0u;
#else
    status.fault_clear = true;
    status.ready = true;
#endif
    status.flags = 0u;
    if (!status.fault_clear) {
        status.flags |= 1u;
    }
    if (!status.ready) {
        status.flags |= 2u;
    }
    return status;
}
