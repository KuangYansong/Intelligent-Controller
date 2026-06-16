#ifndef SSCB_CAN_PROTOCOL_H
#define SSCB_CAN_PROTOCOL_H

#include <stdint.h>
#include "common/can_frame.h"
#include "common/parameters.h"
#include "common/sscb_types.h"

sscb_status_t sscb_can_make_realtime(uint8_t node_id, uint16_t status,
                                     const sscb_measurements_t *m,
                                     sscb_can_frame_t *frame);
sscb_status_t sscb_can_make_fault(uint8_t node_id, sscb_fault_t fault,
                                  sscb_state_t state, int16_t value_main,
                                  uint32_t timestamp_low_ms,
                                  sscb_can_frame_t *frame);
sscb_status_t sscb_can_make_heartbeat(uint8_t node_id, sscb_state_t state,
                                      uint16_t status, sscb_can_frame_t *frame);
sscb_status_t sscb_can_handle_param_request(sscb_params_t *params,
                                            const sscb_can_frame_t *request,
                                            sscb_can_frame_t *response);

#endif
