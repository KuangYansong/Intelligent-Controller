#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>
#include "protocol_service.h"
#include "protection.h"
#include "sscb_types.h"

typedef struct
{
    /* FRAM 中保存的可配置参数，例如阈值、节点号、校准系数。 */
    SscbParams params;
    /* ADC 原始采样值，还没有换算成物理量。 */
    SscbAdcRaw raw;
    /* 已经换算好的电压、电流、温度等测量结果。 */
    SscbMeasurements measurements;
    /* 保护逻辑运行状态，例如是否跳闸、当前故障、恢复过程。 */
    ProtectionService protection;
    /* 协议服务状态，负责处理 CAN 收发。 */
    ProtocolService protocol;
    /* 10ms 控制周期的时间戳。 */
    uint32_t last_10ms;
    /* 实时遥测报文上次发送时间。 */
    uint32_t last_realtime;
    /* 心跳报文上次发送时间。 */
    uint32_t last_heartbeat;
    /* 标记系统是否已经完成初始化。 */
    bool initialized;
} SystemContext;

SscbStatus System_Init(SystemContext *ctx);
void System_RunOnce(SystemContext *ctx);
void System_OnCanRx(const SscbCanFrame *frame);
void System_OnShortTripInterrupt(void);

#endif
