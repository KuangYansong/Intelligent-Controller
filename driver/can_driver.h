#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdbool.h>
#include "can_protocol.h"
#include "sscb_types.h"

typedef void (*CanRxCallback)(const SscbCanFrame *frame);

/* 初始化 CAN 外设，并登记收包回调。 */
SscbStatus CanDriver_Init(CanRxCallback callback);
/* 发送一帧 CAN 报文。 */
SscbStatus CanDriver_Send(const SscbCanFrame *frame);
/* 轮询接收路径；当前项目里主机模式下为占位实现。 */
void CanDriver_Poll(void);

#endif
