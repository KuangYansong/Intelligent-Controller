#include "storage/fault_log.h"

void sscb_fault_log_init(sscb_fault_log_t *log)
{
    uint16_t i;
    if (log == 0) {
        return;
    }
    log->write_index = 0u;
    log->count = 0u;
    log->total_count = 0u;
    for (i = 0u; i < SSCB_FAULT_LOG_CAPACITY; i++) {
        log->records[i].sequence = 0u;
        log->records[i].fault = SSCB_FAULT_NONE;
    }
}

sscb_status_t sscb_fault_log_append(sscb_fault_log_t *log, const sscb_fault_record_t *record)
{
    if (log == 0 || record == 0) {
        return SSCB_ERR_ARG;
    }
    log->records[log->write_index] = *record;
    log->write_index = (uint16_t)((log->write_index + 1u) % SSCB_FAULT_LOG_CAPACITY);
    if (log->count < SSCB_FAULT_LOG_CAPACITY) {
        log->count++;
    }
    log->total_count++;
    return SSCB_OK;
}

uint16_t sscb_fault_log_count(const sscb_fault_log_t *log)
{
    return log == 0 ? 0u : log->count;
}

sscb_status_t sscb_fault_log_get_latest(const sscb_fault_log_t *log, uint16_t latest_index, sscb_fault_record_t *out)
{
    uint16_t pos;
    if (log == 0 || out == 0) {
        return SSCB_ERR_ARG;
    }
    if (latest_index >= log->count) {
        return SSCB_ERR_RANGE;
    }
    pos = (uint16_t)((log->write_index + SSCB_FAULT_LOG_CAPACITY - 1u - latest_index) % SSCB_FAULT_LOG_CAPACITY);
    *out = log->records[pos];
    return SSCB_OK;
}
