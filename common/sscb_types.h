#ifndef SSCB_TYPES_H
#define SSCB_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SSCB_OK = 0,
    SSCB_ERROR = -1,
    SSCB_BAD_PARAM = -2,
    SSCB_CRC_ERROR = -3,
    SSCB_NOT_READY = -4,
    SSCB_NOT_FOUND = -5
} SscbStatus;

typedef enum
{
    SSCB_STATE_NORMAL = 0,
    SSCB_STATE_FAULT_ACTIVE = 1,
    SSCB_STATE_RECOVER_WAIT = 2,
    SSCB_STATE_RECOVER_TRY = 3,
    SSCB_STATE_LOCKOUT = 4
} SscbSystemState;

typedef enum
{
    SSCB_FAULT_NONE = 0x00,
    SSCB_FAULT_SHORT = 0x01,
    SSCB_FAULT_OVERLOAD = 0x02,
    SSCB_FAULT_OVERVOLTAGE = 0x03,
    SSCB_FAULT_OVERTEMP = 0x04,
    SSCB_FAULT_SAMPLE = 0x05,
    SSCB_FAULT_COMM_TIMEOUT = 0x06,
    SSCB_FAULT_SELFTEST = 0x07
} SscbFaultCode;

typedef enum
{
    SSCB_PARAM_SHORT_THRESHOLD = 0x01,
    SSCB_PARAM_OVERCURRENT_THRESHOLD = 0x02,
    SSCB_PARAM_OVERVOLTAGE_THRESHOLD = 0x03,
    SSCB_PARAM_OVERTEMP_THRESHOLD = 0x04,
    SSCB_PARAM_I2T_THRESHOLD = 0x05,
    SSCB_PARAM_VOLTAGE_K = 0x06,
    SSCB_PARAM_CURRENT_K = 0x07,
    SSCB_PARAM_TEMP_K = 0x08,
    SSCB_PARAM_NODE_ID = 0x09,
    SSCB_PARAM_RECOVER_MODE = 0x0A
} SscbParamId;

typedef struct
{
    uint16_t voltage_raw;
    uint16_t current_raw;
    uint16_t temp_raw;
} SscbAdcRaw;

typedef struct
{
    float voltage_v;
    float current_a;
    float current_rms_a;
    float temperature_c;
} SscbMeasurements;

typedef struct
{
    SscbFaultCode fault;
    uint32_t timestamp_ms;
    float voltage_v;
    float current_a;
    float temperature_c;
} SscbFaultRecord;

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint8_t node_id;
    uint8_t recover_mode;
    float short_threshold_a;
    float overcurrent_threshold_a;
    float overvoltage_threshold_v;
    float overtemp_threshold_c;
    float i2t_threshold;
    float voltage_k;
    float current_k;
    float temp_k;
    uint16_t crc16;
} SscbParams;

#endif

