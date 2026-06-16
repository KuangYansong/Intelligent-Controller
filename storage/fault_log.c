#include "storage/fault_log.h"
#include <string.h>

#define SSCB_FAULT_LOG_MAGIC 0x53464C47u
#define SSCB_FAULT_LOG_VERSION 0x0001u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    uint32_t crc32;
    sscb_fault_log_t log;
} sscb_fault_log_image_t;

static uint32_t fnv1a32(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    uint32_t hash = 2166136261u;
    for (i = 0u; i < length; i++) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static void make_image(sscb_fault_log_image_t *image, const sscb_fault_log_t *log)
{
    image->magic = SSCB_FAULT_LOG_MAGIC;
    image->version = SSCB_FAULT_LOG_VERSION;
    image->length = (uint16_t)sizeof(sscb_fault_log_t);
    image->log = *log;
    image->crc32 = fnv1a32((const uint8_t *)&image->log, image->length);
}

static int valid_image(const sscb_fault_log_image_t *image)
{
    if (image->magic != SSCB_FAULT_LOG_MAGIC || image->version != SSCB_FAULT_LOG_VERSION ||
        image->length != sizeof(sscb_fault_log_t)) {
        return 0;
    }
    return image->crc32 == fnv1a32((const uint8_t *)&image->log, image->length);
}

void sscb_fault_log_init(sscb_fault_log_t *log)
{
    if (log == 0) {
        return;
    }
    sscb_fault_log_clear(log);
}

void sscb_fault_log_clear(sscb_fault_log_t *log)
{
    uint16_t i;
    if (log == 0) {
        return;
    }
    log->write_index = 0u;
    log->count = 0u;
    log->total_count = 0u;
    for (i = 0u; i < SSCB_FAULT_LOG_CAPACITY; i++) {
        memset(&log->records[i], 0, sizeof(log->records[i]));
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

sscb_status_t sscb_fault_log_get_chronological(const sscb_fault_log_t *log, uint16_t index, sscb_fault_record_t *out)
{
    uint16_t pos;

    if (log == 0 || out == 0) {
        return SSCB_ERR_ARG;
    }
    if (index >= log->count) {
        return SSCB_ERR_RANGE;
    }
    pos = (uint16_t)((log->write_index + SSCB_FAULT_LOG_CAPACITY - log->count + index) % SSCB_FAULT_LOG_CAPACITY);
    *out = log->records[pos];
    return SSCB_OK;
}

sscb_status_t sscb_fault_log_save_to_fram(sscb_fram_t *fram, sscb_fault_log_t *log)
{
    sscb_fault_log_image_t image;

    if (fram == 0 || log == 0) {
        return SSCB_ERR_ARG;
    }
    make_image(&image, log);
    return sscb_fram_write(fram, SSCB_FRAM_FAULT_LOG_ADDR, (const uint8_t *)&image, (uint16_t)sizeof(image));
}

sscb_status_t sscb_fault_log_load_from_fram(const sscb_fram_t *fram, sscb_fault_log_t *log)
{
    sscb_fault_log_image_t image;
    sscb_status_t rc;

    if (fram == 0 || log == 0) {
        return SSCB_ERR_ARG;
    }

    rc = sscb_fram_read(fram, SSCB_FRAM_FAULT_LOG_ADDR, (uint8_t *)&image, (uint16_t)sizeof(image));
    if (rc != SSCB_OK || !valid_image(&image)) {
        sscb_fault_log_clear(log);
        (void)sscb_fault_log_save_to_fram((sscb_fram_t *)fram, log);
        return rc == SSCB_OK ? SSCB_ERR_NOT_FOUND : rc;
    }

    *log = image.log;
    return SSCB_OK;
}
