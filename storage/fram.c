#include "storage/fram.h"
#include "bsp/board_resources.h"
#include "driver/spi_driver.h"
#ifndef SSCB_TARGET_C2000
#include <string.h>
#endif

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

void sscb_fram_init(sscb_fram_t *fram)
{
    if (fram == 0) {
        return;
    }
    fram->initialized = 1u;
#ifndef SSCB_TARGET_C2000
    memset(fram->memory, 0xFF, sizeof(fram->memory));
#endif
}

sscb_status_t sscb_fram_hw_init(void)
{
    return sscb_spi_driver_init();
}

static sscb_status_t fram_spi_command(uint8_t command, uint32_t address,
                                      const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    uint8_t hdr[4];
    sscb_status_t rc;

    hdr[0] = command;
    hdr[1] = (uint8_t)((address >> 16) & 0xFFu);
    hdr[2] = (uint8_t)((address >> 8) & 0xFFu);
    hdr[3] = (uint8_t)(address & 0xFFu);

    rc = sscb_spi_transfer(hdr, 0, 4u);
    if (rc != SSCB_OK) {
        return rc;
    }
    return sscb_spi_transfer(tx, rx, len);
}

sscb_status_t sscb_fram_read(const sscb_fram_t *fram, uint32_t address, uint8_t *data, uint16_t len)
{
    if (fram == 0 || data == 0 || fram->initialized == 0u ||
        address >= SSCB_FRAM_MEMORY_SIZE || len > SSCB_FRAM_MEMORY_SIZE - address) {
        return SSCB_ERR_ARG;
    }
#ifndef SSCB_TARGET_C2000
    memcpy(data, &fram->memory[address], len);
    return SSCB_OK;
#else
    return fram_spi_command(SSCB_FRAM_CMD_READ, address, 0, data, len);
#endif
}

sscb_status_t sscb_fram_write(sscb_fram_t *fram, uint32_t address, const uint8_t *data, uint16_t len)
{
    uint8_t wren;
    sscb_status_t rc;

    if (fram == 0 || data == 0 || fram->initialized == 0u ||
        address >= SSCB_FRAM_MEMORY_SIZE || len > SSCB_FRAM_MEMORY_SIZE - address) {
        return SSCB_ERR_ARG;
    }
#ifndef SSCB_TARGET_C2000
    memcpy(&fram->memory[address], data, len);
    return SSCB_OK;
#else
    wren = SSCB_FRAM_CMD_WREN;
    rc = sscb_spi_transfer(&wren, 0, 1u);
    if (rc != SSCB_OK) {
        return rc;
    }
    return fram_spi_command(SSCB_FRAM_CMD_WRITE, address, data, 0, len);
#endif
}
