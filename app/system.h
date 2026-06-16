#ifndef SSCB_SYSTEM_H
#define SSCB_SYSTEM_H

#include <stdbool.h>
#include "common/parameters.h"
#include "common/sscb_types.h"
#include "protection/protection.h"
#include "storage/fault_log.h"

typedef struct {
    sscb_params_t params;
    sscb_protection_t protection;
    sscb_fault_log_t fault_log;
    sscb_state_t state;
    sscb_fault_t active_fault;
    uint16_t status_word;
    uint32_t time_ms;
    uint16_t recover_elapsed_ms;
    bool trip_clear;
    bool driver_ready;
    bool fault_conditions_clear;
} sscb_system_t;

void sscb_system_init(sscb_system_t *sys);
sscb_state_t sscb_system_state(const sscb_system_t *sys);
sscb_status_t sscb_system_start(sscb_system_t *sys);
void sscb_system_tick_1ms(sscb_system_t *sys, const sscb_measurements_t *m);
void sscb_system_on_short_trip(sscb_system_t *sys, const sscb_measurements_t *m);
void sscb_system_set_safety_inputs(sscb_system_t *sys, bool trip_clear,
                                   bool driver_ready, bool fault_conditions_clear);
sscb_status_t sscb_system_manual_reset(sscb_system_t *sys);

#endif
