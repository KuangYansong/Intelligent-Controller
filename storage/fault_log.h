#ifndef SSCB_FAULT_LOG_H
#define SSCB_FAULT_LOG_H

#include "common/sscb_types.h"

#define SSCB_FAULT_LOG_CAPACITY 50u

typedef struct {
    uint16_t sequence;
    sscb_fault_t fault;
    sscb_state_t state;
    uint8_t flags;
    uint64_t timestamp_ms;
    uint16_t voltage_dv;
    int16_t current_pga_da;
    int16_t current_raw_da;
    int16_t temperature_dc;
    uint32_t i2t_value;
    uint16_t adc_flags;
    uint16_t driver_flags;
} sscb_fault_record_t;

typedef struct {
    sscb_fault_record_t records[SSCB_FAULT_LOG_CAPACITY];
    uint16_t write_index;
    uint16_t count;
    uint32_t total_count;
} sscb_fault_log_t;

void sscb_fault_log_init(sscb_fault_log_t *log);
sscb_status_t sscb_fault_log_append(sscb_fault_log_t *log, const sscb_fault_record_t *record);
uint16_t sscb_fault_log_count(const sscb_fault_log_t *log);
sscb_status_t sscb_fault_log_get_latest(const sscb_fault_log_t *log, uint16_t latest_index, sscb_fault_record_t *out);

#endif
