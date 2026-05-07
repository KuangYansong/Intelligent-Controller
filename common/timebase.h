#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdbool.h>
#include <stdint.h>

void Timebase_Init(void);
void Timebase_Tick1ms(void);
uint32_t Timebase_NowMs(void);
uint64_t Timebase_NowUnixMs(void);
void Timebase_SyncUnix(uint32_t unix_sec, uint32_t millis_in_sec);
bool Timebase_IsSynced(void);

#endif

