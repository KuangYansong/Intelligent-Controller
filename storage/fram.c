#include "fram.h"
#include "sscb_config.h"

#ifdef SSCB_TARGET_C2000
#include "spi_driver.h"
#endif

#ifndef SSCB_TARGET_C2000
/* 主机测试模式下，用一块内存数组模拟整片 FRAM。 */
static uint8_t s_host_fram[SSCB_FRAM_SIZE_BYTES];
#endif

SscbStatus Fram_Init(void)
{
#ifdef SSCB_TARGET_C2000
    /* 真机上把初始化工作交给 SPI 驱动。 */
    return SpiDriver_Init();
#else
    /* 主机模式没有真实硬件，内存数组天然可用。 */
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
    /* 真机从外部 FRAM 芯片读取。 */
    return SpiDriver_FramRead(address, (uint8_t *)data, len);
#else
    /* 主机模式直接从模拟内存中拷贝。 */
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
    /* 真机写外部 FRAM 芯片。 */
    return SpiDriver_FramWrite(address, (const uint8_t *)data, len);
#else
    /* 主机模式写入模拟内存。 */
    const uint8_t *src = (const uint8_t *)data;
    for (size_t i = 0u; i < len; i++)
    {
        s_host_fram[address + i] = src[i];
    }
    return SSCB_OK;
#endif
}
