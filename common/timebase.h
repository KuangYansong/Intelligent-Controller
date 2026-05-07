#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdbool.h>
#include <stdint.h>

/* 初始化本地运行时间。 */
void Timebase_Init(void);
/* 每经过 1ms 调用一次，用于累加本地时钟。 */
void Timebase_Tick1ms(void);
/* 获取上电以来的本地毫秒计数。 */
uint32_t Timebase_NowMs(void);
/* 获取当前 Unix 毫秒时间；若未同步则退化为本地毫秒。 */
uint64_t Timebase_NowUnixMs(void);
/* 用上位机下发的时间同步本地时钟。 */
void Timebase_SyncUnix(uint32_t unix_sec, uint32_t millis_in_sec);
/* 判断是否已经完成过时间同步。 */
bool Timebase_IsSynced(void);

#endif
