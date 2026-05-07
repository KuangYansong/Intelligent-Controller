#include "spi_driver.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#endif

SscbStatus SpiDriver_Init(void)
{
#ifdef SSCB_TARGET_C2000
    SPI_disableModule(SPIB_BASE);
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA0, SPI_MODE_MASTER, 10000000u, 8u);
    SPI_enableModule(SPIB_BASE);
#endif
    return SSCB_OK;
}

SscbStatus SpiDriver_FramRead(uint32_t address, uint8_t *data, size_t len)
{
    (void)address;
    (void)data;
    (void)len;
    return SSCB_NOT_READY;
}

SscbStatus SpiDriver_FramWrite(uint32_t address, const uint8_t *data, size_t len)
{
    (void)address;
    (void)data;
    (void)len;
    return SSCB_NOT_READY;
}

