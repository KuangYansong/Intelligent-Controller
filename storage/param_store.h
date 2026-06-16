#ifndef SSCB_PARAM_STORE_H
#define SSCB_PARAM_STORE_H

#include "common/parameters.h"

#define SSCB_PARAM_STORE_MAGIC 0x53534342u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
    sscb_params_t params;
} sscb_param_image_t;

typedef struct {
    sscb_param_image_t slot_a;
    sscb_param_image_t slot_b;
} sscb_param_store_t;

void sscb_param_store_init(sscb_param_store_t *store);
sscb_status_t sscb_param_store_save(sscb_param_store_t *store, const sscb_params_t *params);
sscb_status_t sscb_param_store_load(sscb_param_store_t *store, sscb_params_t *params);

#endif
