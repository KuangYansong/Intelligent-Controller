#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include "sscb_types.h"

#define SSCB_CAN_ID_REALTIME_BASE  (0x100u)
#define SSCB_CAN_ID_FAULT_BASE     (0x200u)
#define SSCB_CAN_ID_HEARTBEAT_BASE (0x300u)
#define SSCB_CAN_ID_PARAM_BASE     (0x400u)
#define SSCB_CAN_ID_TIME_BASE      (0x500u)
#define SSCB_CAN_ID_CONTROL_BASE   (0x600u)

typedef struct
{
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
} SscbCanFrame;

typedef enum
{
    SSCB_CTRL_CLEAR_FAULT = 0x01,
    SSCB_CTRL_SOFTWARE_RESET = 0x02,
    SSCB_CTRL_READ_FAULT_LOG = 0x03,
    SSCB_CTRL_CLEAR_FAULT_LOG = 0x04
} SscbControlCommand;

uint16_t CanProtocol_MakeId(uint16_t base, uint8_t node_id);
uint8_t CanProtocol_StatusByte(SscbSystemState state, uint32_t fault_bits, bool selftest_failed);
SscbCanFrame CanProtocol_RealtimeFrame(uint8_t node_id, const SscbMeasurements *m, SscbSystemState state, uint32_t fault_bits, bool selftest_failed);
SscbCanFrame CanProtocol_FaultFrame(uint8_t node_id, const SscbFaultRecord *record);
SscbCanFrame CanProtocol_HeartbeatFrame(uint8_t node_id, SscbSystemState state, uint64_t runtime_s);
SscbCanFrame CanProtocol_ParamResponse(uint8_t node_id, SscbStatus status, SscbParamId id, float value);
bool CanProtocol_IsForNode(const SscbCanFrame *frame, uint16_t base, uint8_t node_id);
bool CanProtocol_ParseTimeSync(const SscbCanFrame *frame, uint8_t node_id, uint32_t *unix_sec, uint32_t *millis);
bool CanProtocol_ParseControl(const SscbCanFrame *frame, uint8_t node_id, SscbControlCommand *cmd);
bool CanProtocol_ParseParamRequest(const SscbCanFrame *frame, uint8_t node_id, bool *write, SscbParamId *id, float *value);

#endif

