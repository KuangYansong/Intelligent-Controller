#ifndef SSCB_FIRMWARE_H
#define SSCB_FIRMWARE_H

#include "app/system.h"
#include "common/timebase.h"
#include "driver/gate_driver.h"
#include "storage/fram.h"
#include "storage/param_store.h"

typedef struct {
    sscb_system_t system;
    sscb_timebase_t timebase;
    sscb_fram_t fram;
    sscb_param_store_t param_store;
    sscb_gate_driver_config_t gate_driver_config;
    uint16_t realtime_elapsed_ms;
    uint16_t heartbeat_elapsed_ms;
    uint32_t fault_log_saved_total_count;
    uint32_t fault_log_reported_total_count;
} sscb_firmware_t;

sscb_status_t sscb_firmware_init(sscb_firmware_t *fw);
void sscb_firmware_run_once(sscb_firmware_t *fw);

#endif
