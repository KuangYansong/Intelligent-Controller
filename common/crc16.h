#ifndef SSCB_CRC16_H
#define SSCB_CRC16_H

#include "common/sscb_types.h"

uint16_t sscb_crc16_ccitt(const uint8_t *data, uint16_t length);

#endif
