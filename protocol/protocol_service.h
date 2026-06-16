#ifndef SSCB_PROTOCOL_SERVICE_H
#define SSCB_PROTOCOL_SERVICE_H

#include "app/system.h"
#include "common/can_frame.h"

sscb_status_t sscb_protocol_handle_frame(sscb_system_t *sys,
                                         const sscb_can_frame_t *request,
                                         sscb_can_frame_t *response);

#endif
