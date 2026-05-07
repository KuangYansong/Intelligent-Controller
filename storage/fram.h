#ifndef FRAM_H
#define FRAM_H

#include <stddef.h>
#include <stdint.h>
#include "sscb_types.h"

SscbStatus Fram_Init(void);
SscbStatus Fram_Read(uint32_t address, void *data, size_t len);
SscbStatus Fram_Write(uint32_t address, const void *data, size_t len);

#endif

