#ifndef PROTECTION_H
#define PROTECTION_H

#include <stdbool.h>
#include <stdint.h>
#include "i2t.h"
#include "rms.h"
#include "sscb_types.h"

typedef void (*ProtectionTripOutput)(bool enable);

typedef struct
{
    SscbSystemState state;
    SscbFaultCode active_fault;
    uint32_t fault_bits;
    uint32_t fault_started_ms;
    uint32_t recover_started_ms;
    bool auto_recover_used;
    bool selftest_failed;
    RmsWindow rms;
    I2tState i2t;
    ProtectionTripOutput trip_output;
} ProtectionService;

void Protection_Init(ProtectionService *svc, ProtectionTripOutput trip_output);
float Protection_AddCurrentSample(ProtectionService *svc, float current_a);
SscbFaultCode Protection_Evaluate10ms(ProtectionService *svc, const SscbParams *params, SscbMeasurements *m, uint32_t now_ms);
SscbFaultCode Protection_OnShortTrip(ProtectionService *svc, const SscbMeasurements *m, uint32_t now_ms);
bool Protection_UpdateRecovery(ProtectionService *svc, const SscbParams *params, const SscbMeasurements *m, uint32_t now_ms);
void Protection_ClearFaults(ProtectionService *svc, const SscbMeasurements *m);
bool Protection_IsTripped(const ProtectionService *svc);

#endif

