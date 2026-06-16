#include "common/timebase.h"

void sscb_timebase_init(sscb_timebase_t *tb)
{
    if (tb == 0) {
        return;
    }
    tb->unix_sec = 0u;
    tb->local_ms = 0u;
    tb->sync_local_ms = 0u;
    tb->synced = 0u;
}

void sscb_timebase_tick_ms(sscb_timebase_t *tb)
{
    if (tb != 0) {
        tb->local_ms++;
    }
}

void sscb_timebase_sync(sscb_timebase_t *tb, uint32_t unix_sec, uint32_t millisec)
{
    if (tb == 0) {
        return;
    }
    tb->unix_sec = unix_sec;
    tb->sync_local_ms = tb->local_ms - millisec;
    tb->synced = 1u;
}

uint64_t sscb_timebase_absolute_ms(const sscb_timebase_t *tb)
{
    if (tb == 0 || tb->synced == 0u) {
        return tb == 0 ? 0u : tb->local_ms;
    }
    return (uint64_t)tb->unix_sec * 1000u + (uint64_t)(tb->local_ms - tb->sync_local_ms);
}
