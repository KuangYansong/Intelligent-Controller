#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdbool.h>
#include "can_protocol.h"
#include "sscb_types.h"

typedef void (*CanRxCallback)(const SscbCanFrame *frame);

SscbStatus CanDriver_Init(CanRxCallback callback);
SscbStatus CanDriver_Send(const SscbCanFrame *frame);
void CanDriver_Poll(void);

#endif

