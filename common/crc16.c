#include "crc16.h"

uint16_t Crc16_CcittFalse(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint16_t crc = 0xFFFFu;

    while (len-- > 0u)
    {
        crc ^= (uint16_t)(*bytes++) << 8;
        for (uint8_t bit = 0u; bit < 8u; bit++)
        {
            if ((crc & 0x8000u) != 0u)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else
            {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}

