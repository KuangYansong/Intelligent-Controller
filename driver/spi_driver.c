#include "driver/spi_driver.h"
#include "bsp/board_resources.h"

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

sscb_status_t sscb_spi_driver_init(void)
{
#ifdef __TMS320C28XX__
    SPI_disableModule(SPIB_BASE);
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA0,
                  SPI_MODE_CONTROLLER, SSCB_SPIB_BITRATE, 8u);
    SPI_disableFIFO(SPIB_BASE);
    SPI_setEmulationMode(SPIB_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableModule(SPIB_BASE);
#endif
    return SSCB_OK;
}

sscb_status_t sscb_spi_transfer(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    uint16_t i;
    if (len != 0u && tx == 0 && rx == 0) {
        return SSCB_ERR_ARG;
    }
    for (i = 0u; i < len; i++) {
#ifdef __TMS320C28XX__
        uint16_t data = tx == 0 ? 0xFFu : tx[i];
        SPI_writeDataBlockingNonFIFO(SPIB_BASE, data);
        data = SPI_readDataBlockingNonFIFO(SPIB_BASE);
        if (rx != 0) {
            rx[i] = (uint8_t)data;
        }
#else
        if (rx != 0) {
            rx[i] = tx == 0 ? 0xFFu : tx[i];
        }
#endif
    }
    return SSCB_OK;
}
