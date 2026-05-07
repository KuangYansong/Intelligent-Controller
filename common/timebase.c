#include "timebase.h"

static volatile uint32_t s_local_ms;
static uint64_t s_unix_base_ms;
static uint32_t s_sync_local_ms;
static bool s_synced;

void Timebase_Init(void)
{
    s_local_ms = 0u;
    s_unix_base_ms = 0u;
    s_sync_local_ms = 0u;
    s_synced = false;
}

void Timebase_Tick1ms(void)
{
    s_local_ms++;
}

uint32_t Timebase_NowMs(void)
{
    return s_local_ms;
}

uint64_t Timebase_NowUnixMs(void)
{
    if (!s_synced)
    {
        return (uint64_t)s_local_ms;
    }

    return s_unix_base_ms + (uint32_t)(s_local_ms - s_sync_local_ms);
}

void Timebase_SyncUnix(uint32_t unix_sec, uint32_t millis_in_sec)
{
    s_unix_base_ms = ((uint64_t)unix_sec * 1000ull) + (uint64_t)millis_in_sec;
    s_sync_local_ms = s_local_ms;
    s_synced = true;
}

bool Timebase_IsSynced(void)
{
    return s_synced;
}

