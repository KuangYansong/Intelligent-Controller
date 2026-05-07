#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stddef.h>
#include <stdint.h>
#include "sscb_types.h"

SscbStatus SpiDriver_Init(void);
SscbStatus SpiDriver_FramRead(uint32_t address, uint8_t *data, size_t len);
SscbStatus SpiDriver_FramWrite(uint32_t address, const uint8_t *data, size_t len);

#endif

