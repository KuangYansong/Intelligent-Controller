#include "timebase.h"

static volatile uint32_t s_local_ms;
static uint64_t s_unix_base_ms;
static uint32_t s_sync_local_ms;
static bool s_synced;

void Timebase_Init(void)
{
    /* 本地时间从 0 开始，初始时尚未与真实世界时间同步。 */
    s_local_ms = 0u;
    s_unix_base_ms = 0u;
    s_sync_local_ms = 0u;
    s_synced = false;
}

void Timebase_Tick1ms(void)
{
    /* 这个函数通常应该由 1ms 定时中断驱动。 */
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
        /* 没同步过时，先退化成本地运行毫秒数。 */
        return (uint64_t)s_local_ms;
    }

    /* 真实时间 = 同步时刻的 Unix 毫秒 + 同步后流逝的本地毫秒。 */
    return s_unix_base_ms + (uint32_t)(s_local_ms - s_sync_local_ms);
}

void Timebase_SyncUnix(uint32_t unix_sec, uint32_t millis_in_sec)
{
    /* 记录“当前本地毫秒”对应的真实 Unix 时间基准。 */
    s_unix_base_ms = ((uint64_t)unix_sec * 1000ull) + (uint64_t)millis_in_sec;
    s_sync_local_ms = s_local_ms;
    s_synced = true;
}

bool Timebase_IsSynced(void)
{
    return s_synced;
}
