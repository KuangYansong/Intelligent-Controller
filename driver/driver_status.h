#ifndef SSCB_DRIVER_STATUS_H
#define SSCB_DRIVER_STATUS_H

#include <stdbool.h>
#include <stdint.h>
#include "common/sscb_types.h"

typedef struct {
    bool fault_clear;
    bool ready;
    uint16_t flags;
} sscb_driver_status_t;

sscb_status_t sscb_driver_status_init(void);
sscb_driver_status_t sscb_driver_status_read(void);

#endif
