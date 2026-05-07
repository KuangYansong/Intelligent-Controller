#include "fram.h"
#include "sscb_config.h"

#ifdef SSCB_TARGET_C2000
#include "spi_driver.h"
#endif

#ifndef SSCB_TARGET_C2000
static uint8_t s_host_fram[SSCB_FRAM_SIZE_BYTES];
#endif

SscbStatus Fram_Init(void)
{
#ifdef SSCB_TARGET_C2000
    return SpiDriver_Init();
#else
    return SSCB_OK;
#endif
}

SscbStatus Fram_Read(uint32_t address, void *data, size_t len)
{
    if ((data == 0) || ((address + len) > SSCB_FRAM_SIZE_BYTES))
    {
        return SSCB_BAD_PARAM;
    }

#ifdef SSCB_TARGET_C2000
    return SpiDriver_FramRead(address, (uint8_t *)data, len);
#else
    uint8_t *dst = (uint8_t *)data;
    for (size_t i = 0u; i < len; i++)
    {
        dst[i] = s_host_fram[address + i];
    }
    return SSCB_OK;
#endif
}

SscbStatus Fram_Write(uint32_t address, const void *data, size_t len)
{
    if ((data == 0) || ((address + len) > SSCB_FRAM_SIZE_BYTES))
    {
        return SSCB_BAD_PARAM;
    }

#ifdef SSCB_TARGET_C2000
    return SpiDriver_FramWrite(address, (const uint8_t *)data, len);
#else
    const uint8_t *src = (const uint8_t *)data;
    for (size_t i = 0u; i < len; i++)
    {
        s_host_fram[address + i] = src[i];
    }
    return SSCB_OK;
#endif
}

