#ifndef SSCB_CAN_DRIVER_H
#define SSCB_CAN_DRIVER_H

#include "common/can_frame.h"
#include "common/sscb_types.h"

sscb_status_t sscb_can_driver_init(uint32_t bitrate);
sscb_status_t sscb_can_driver_send(const sscb_can_frame_t *frame);
sscb_status_t sscb_can_driver_receive(sscb_can_frame_t *frame);

#endif
