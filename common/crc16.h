#ifndef CRC16_H
#define CRC16_H

#include <stddef.h>
#include <stdint.h>

/* 计算 CRC16/CCITT-FALSE，常用于参数块完整性校验。 */
uint16_t Crc16_CcittFalse(const void *data, size_t len);

#endif
