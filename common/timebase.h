#ifndef SSCB_TIMEBASE_H
#define SSCB_TIMEBASE_H

#include "common/sscb_types.h"

typedef struct {
    uint32_t unix_sec;
    uint32_t local_ms;
    uint32_t sync_local_ms;
    uint8_t synced;
} sscb_timebase_t;

void sscb_timebase_init(sscb_timebase_t *tb);
void sscb_timebase_tick_ms(sscb_timebase_t *tb);
void sscb_timebase_sync(sscb_timebase_t *tb, uint32_t unix_sec, uint32_t millisec);
uint64_t sscb_timebase_absolute_ms(const sscb_timebase_t *tb);

#endif
