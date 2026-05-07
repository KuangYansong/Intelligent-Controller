#ifndef PROTECTION_H
#define PROTECTION_H

#include <stdbool.h>
#include <stdint.h>
#include "i2t.h"
#include "rms.h"
#include "sscb_types.h"

typedef void (*ProtectionTripOutput)(bool enable);

typedef struct
{
    /* 当前保护状态，例如正常、故障中、等待恢复、锁定。 */
    SscbSystemState state;
    /* 当前正在处理的故障类型。 */
    SscbFaultCode active_fault;
    /* 历史故障位图，每一位代表一种故障是否出现过。 */
    uint32_t fault_bits;
    /* 故障开始时间，用于计算延时恢复。 */
    uint32_t fault_started_ms;
    /* 恢复流程开始时间。 */
    uint32_t recover_started_ms;
    /* 是否已经使用过一次自动恢复机会。 */
    bool auto_recover_used;
    /* 自检失败标志。 */
    bool selftest_failed;
    /* 电流有效值计算窗口。 */
    RmsWindow rms;
    /* I2T 积分状态。 */
    I2tState i2t;
    /* 控制外部跳闸输出的回调函数。 */
    ProtectionTripOutput trip_output;
} ProtectionService;

void Protection_Init(ProtectionService *svc, ProtectionTripOutput trip_output);
float Protection_AddCurrentSample(ProtectionService *svc, float current_a);
SscbFaultCode Protection_Evaluate10ms(ProtectionService *svc, const SscbParams *params, SscbMeasurements *m, uint32_t now_ms);
SscbFaultCode Protection_OnShortTrip(ProtectionService *svc, const SscbMeasurements *m, uint32_t now_ms);
bool Protection_UpdateRecovery(ProtectionService *svc, const SscbParams *params, const SscbMeasurements *m, uint32_t now_ms);
void Protection_ClearFaults(ProtectionService *svc, const SscbMeasurements *m);
bool Protection_IsTripped(const ProtectionService *svc);

#endif
