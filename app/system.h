#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>
#include "protocol_service.h"
#include "protection.h"
#include "sscb_types.h"

typedef struct
{
    SscbParams params;
    SscbAdcRaw raw;
    SscbMeasurements measurements;
    ProtectionService protection;
    ProtocolService protocol;
    uint32_t last_10ms;
    uint32_t last_realtime;
    uint32_t last_heartbeat;
    bool initialized;
} SystemContext;

SscbStatus System_Init(SystemContext *ctx);
void System_RunOnce(SystemContext *ctx);
void System_OnCanRx(const SscbCanFrame *frame);
void System_OnShortTripInterrupt(void);

#endif

