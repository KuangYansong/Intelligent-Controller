#ifndef PROTOCOL_SERVICE_H
#define PROTOCOL_SERVICE_H

#include "can_protocol.h"
#include "sscb_types.h"

typedef void (*ProtocolFrameTx)(const SscbCanFrame *frame);
typedef void (*ProtocolControlHandler)(SscbControlCommand cmd);

typedef struct
{
    SscbParams *params;
    ProtocolFrameTx tx;
    ProtocolControlHandler control_handler;
} ProtocolService;

void ProtocolService_Init(ProtocolService *svc, SscbParams *params, ProtocolFrameTx tx, ProtocolControlHandler control_handler);
void ProtocolService_OnRx(ProtocolService *svc, const SscbCanFrame *frame);
void ProtocolService_SendRealtime(ProtocolService *svc, const SscbMeasurements *m, SscbSystemState state, uint32_t fault_bits, bool selftest_failed);
void ProtocolService_SendFault(ProtocolService *svc, const SscbFaultRecord *record);
void ProtocolService_SendHeartbeat(ProtocolService *svc, SscbSystemState state, uint64_t runtime_s);

#endif

