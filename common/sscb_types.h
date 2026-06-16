#ifndef SSCB_TYPES_H
#define SSCB_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#if defined(__TMS320C2000__)
#include "inc/hw_types.h"
#ifndef UINT8_MAX
#define UINT8_MAX 0x00FFu
#define INT8_MAX 0x007F
#define INT8_MIN (-0x0080)
#endif
#endif

typedef enum {
    SSCB_OK = 0,
    SSCB_ERR_RANGE = -1,
    SSCB_ERR_ARG = -2,
    SSCB_ERR_STATE = -3,
    SSCB_ERR_NOT_FOUND = -4,
    SSCB_ERR_TIMEOUT = -5,
    SSCB_ERR_HW = -6
} sscb_status_t;

typedef enum {
    SSCB_STATE_INIT = 0x00,
    SSCB_STATE_SELF_TEST = 0x01,
    SSCB_STATE_READY = 0x02,
    SSCB_STATE_RUN = 0x03,
    SSCB_STATE_WARN = 0x04,
    SSCB_STATE_FAULT_ACTIVE = 0x05,
    SSCB_STATE_RECOVER_WAIT = 0x06,
    SSCB_STATE_RECOVER_TRY = 0x07,
    SSCB_STATE_LOCKOUT = 0x08,
    SSCB_STATE_MAINTENANCE = 0x09
} sscb_state_t;

typedef enum {
    SSCB_FAULT_NONE = 0x00,
    SSCB_FAULT_SHORT = 0x01,
    SSCB_FAULT_OVERLOAD = 0x02,
    SSCB_FAULT_OVERVOLTAGE = 0x03,
    SSCB_FAULT_OVERTEMP_WARN = 0x04,
    SSCB_FAULT_SAMPLE_ABNORMAL = 0x05,
    SSCB_FAULT_DRIVER = 0x06,
    SSCB_FAULT_PARAM_CRC = 0x07,
    SSCB_FAULT_COMM_TIMEOUT = 0x08
} sscb_fault_t;

enum {
    SSCB_STATUS_NORMAL = 1u << 0,
    SSCB_STATUS_SHORT = 1u << 1,
    SSCB_STATUS_OVERLOAD = 1u << 2,
    SSCB_STATUS_OVERVOLTAGE = 1u << 3,
    SSCB_STATUS_OVERTEMP_WARN = 1u << 4,
    SSCB_STATUS_LOCKOUT = 1u << 5,
    SSCB_STATUS_SELFTEST_FAIL = 1u << 6,
    SSCB_STATUS_SAMPLE_ABNORMAL = 1u << 7,
    SSCB_STATUS_DRIVER_FAULT = 1u << 8,
    SSCB_STATUS_COMM_ABNORMAL = 1u << 9,
    SSCB_STATUS_PGA_SATURATED = 1u << 10,
    SSCB_STATUS_B3_VALID = 1u << 11
};

typedef struct {
    uint16_t voltage_dv;
    int16_t current_pga_da;
    int16_t current_raw_da;
    int16_t temperature_dc;
    uint16_t adc_flags;
    uint16_t driver_flags;
} sscb_measurements_t;

typedef struct {
    uint32_t ms;
    uint32_t unix_sec;
} sscb_time_t;

#endif
