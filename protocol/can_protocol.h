#ifndef SSCB_CAN_PROTOCOL_H
#define SSCB_CAN_PROTOCOL_H

#include <stdint.h>
#include "common/can_frame.h"
#include "common/parameters.h"
#include "common/timebase.h"
#include "common/sscb_types.h"
#include "storage/fault_log.h"

sscb_status_t sscb_can_make_realtime(uint8_t node_id, uint16_t status,
                                     const sscb_measurements_t *m,
                                     sscb_can_frame_t *frame);
sscb_status_t sscb_can_make_fault(uint8_t node_id, sscb_fault_t fault,
                                  uint32_t timestamp_ms,
                                  int16_t current_pga_da,
                                  uint16_t voltage_dv,
                                  sscb_can_frame_t *frame);
sscb_status_t sscb_can_make_heartbeat(uint8_t node_id, sscb_state_t state,
                                      uint64_t runtime_seconds, sscb_can_frame_t *frame);
sscb_status_t sscb_can_handle_param_request(sscb_params_t *params,
                                            const sscb_can_frame_t *request,
                                            sscb_can_frame_t *response);
sscb_status_t sscb_can_handle_time_sync_request(sscb_timebase_t *timebase,
                                                const sscb_can_frame_t *request,
                                                sscb_can_frame_t *response);
sscb_status_t sscb_can_handle_fault_log_request(uint8_t node_id,
                                                sscb_fault_log_t *log,
                                                const sscb_can_frame_t *request,
                                                sscb_can_frame_t *response);

#endif
