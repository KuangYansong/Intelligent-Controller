#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

static inline uint16_t Sscb_ClampU16(float value, float scale)
{
    /* 先缩放，再限制到 uint16 能表示的范围。 */
    float scaled = value * scale;
    if (scaled <= 0.0f)
    {
        return 0u;
    }
    if (scaled >= 65535.0f)
    {
        return 65535u;
    }
    return (uint16_t)(scaled + 0.5f);
}

static inline int16_t Sscb_ClampI16(float value, float scale)
{
    /* 有符号量既要缩放，也要考虑负数四舍五入。 */
    float scaled = value * scale;
    if (scaled <= -32768.0f)
    {
        return -32768;
    }
    if (scaled >= 32767.0f)
    {
        return 32767;
    }
    return (int16_t)(scaled + ((scaled >= 0.0f) ? 0.5f : -0.5f));
}

static inline void Sscb_PutLe16(uint8_t *dst, uint16_t value)
{
    /* 按 little-endian 顺序写入 16 位整数。 */
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static inline void Sscb_PutLe32(uint8_t *dst, uint32_t value)
{
    /* 按 little-endian 顺序写入 32 位整数。 */
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8) & 0xFFu);
    dst[2] = (uint8_t)((value >> 16) & 0xFFu);
    dst[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static inline uint16_t Sscb_GetLe16(const uint8_t *src)
{
    /* 从 little-endian 字节流恢复 16 位整数。 */
    return (uint16_t)src[0] | ((uint16_t)src[1] << 8);
}

static inline uint32_t Sscb_GetLe32(const uint8_t *src)
{
    /* 从 little-endian 字节流恢复 32 位整数。 */
    return (uint32_t)src[0] |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

#endif
