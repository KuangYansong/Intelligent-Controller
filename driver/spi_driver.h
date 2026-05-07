#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stddef.h>
#include <stdint.h>
#include "sscb_types.h"

/* 初始化 SPIB，用于连接外部 FRAM。 */
SscbStatus SpiDriver_Init(void);
/* 通过 SPI 从 FRAM 读取数据。 */
SscbStatus SpiDriver_FramRead(uint32_t address, uint8_t *data, size_t len);
/* 通过 SPI 向 FRAM 写入数据。 */
SscbStatus SpiDriver_FramWrite(uint32_t address, const uint8_t *data, size_t len);

#endif
