#ifndef SSCB_FRAM_H
#define SSCB_FRAM_H

#include "common/sscb_types.h"

#define SSCB_FRAM_MEMORY_SIZE 0x40000ul
#define SSCB_FRAM_PARAM_STORE_ADDR 0x00100ul
#define SSCB_FRAM_FAULT_LOG_ADDR 0x01100ul

typedef struct {
    uint8_t initialized;
#ifndef SSCB_TARGET_C2000
    uint8_t memory[SSCB_FRAM_MEMORY_SIZE];
#endif
} sscb_fram_t;

void sscb_fram_init(sscb_fram_t *fram);
sscb_status_t sscb_fram_hw_init(void);
sscb_status_t sscb_fram_read(const sscb_fram_t *fram, uint32_t address, uint8_t *data, uint16_t len);
sscb_status_t sscb_fram_write(sscb_fram_t *fram, uint32_t address, const uint8_t *data, uint16_t len);

#endif
