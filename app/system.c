#include "system.h"
#include "adc_driver.h"
#include "board.h"
#include "can_driver.h"
#include "cmpss_driver.h"
#include "epwm_driver.h"
#include "fault_log.h"
#include "param_store.h"
#include "selftest.h"
#include "timebase.h"

static SystemContext *s_ctx;

static void tx_frame(const SscbCanFrame *frame)
{
    /* 协议层不直接依赖底层 CAN 驱动，而是通过这个发送回调转出去。 */
    (void)CanDriver_Send(frame);
}

static SscbFaultRecord make_fault_record(SscbFaultCode fault, const SscbMeasurements *m)
{
    SscbFaultRecord record;
    /* 故障记录本质上是“故障类型 + 发生时刻 + 当时测量值”的快照。 */
    record.fault = fault;
    record.timestamp_ms = (uint32_t)Timebase_NowUnixMs();
    record.voltage_v = m->voltage_v;
    record.current_a = m->current_a;
    record.temperature_c = m->temperature_c;
    return record;
}

static void publish_fault(SystemContext *ctx, SscbFaultCode fault)
{
    /* 没有故障时直接返回，避免重复写日志和重复上报。 */
    if ((ctx == 0) || (fault == SSCB_FAULT_NONE))
    {
        return;
    }

    SscbFaultRecord record = make_fault_record(fault, &ctx->measurements);
    (void)FaultLog_Append(&record);
    ProtocolService_SendFault(&ctx->protocol, &record);
}

static void control_handler(SscbControlCommand cmd)
{
    /* 该函数专门响应上位机下发的控制命令。 */
    if (s_ctx == 0)
    {
        return;
    }

    if (cmd == SSCB_CTRL_CLEAR_FAULT)
    {
        Protection_ClearFaults(&s_ctx->protection, &s_ctx->measurements);
    }
    else if (cmd == SSCB_CTRL_SOFTWARE_RESET)
    {
        Board_SoftwareReset();
    }
    else if (cmd == SSCB_CTRL_READ_FAULT_LOG)
    {
        /* 逐条读取故障日志，再通过协议层重新发给上位机。 */
        uint8_t count = FaultLog_Count();
        for (uint8_t i = 0u; i < count; i++)
        {
            SscbFaultRecord record;
            if (FaultLog_Read(i, &record) == SSCB_OK)
            {
                ProtocolService_SendFault(&s_ctx->protocol, &record);
            }
        }
    }
    else if (cmd == SSCB_CTRL_CLEAR_FAULT_LOG)
    {
        (void)FaultLog_Clear();
    }
}

SscbStatus System_Init(SystemContext *ctx)
{
    if (ctx == 0)
    {
        return SSCB_BAD_PARAM;
    }

    /* 保存全局上下文指针，供中断和回调函数访问当前系统状态。 */
    s_ctx = ctx;
    Timebase_Init();

    /* 初始化板级资源，包括 ADC、PWM、FRAM 等底层外设。 */
    if (Board_Init() != SSCB_OK)
    {
        return SSCB_ERROR;
    }
    /* 尝试从 FRAM 读取配置；失败则恢复默认值并保存。 */
    if (ParamStore_Load(&ctx->params) != SSCB_OK)
    {
        ParamStore_Defaults(&ctx->params);
        (void)ParamStore_Save(&ctx->params);
    }
    if (FaultLog_Init() != SSCB_OK)
    {
        return SSCB_ERROR;
    }
    if (CmpssDriver_Init(ctx->params.short_threshold_a) != SSCB_OK)
    {
        return SSCB_ERROR;
    }
    if (CanDriver_Init(System_OnCanRx) != SSCB_OK)
    {
        return SSCB_ERROR;
    }

    /* 先采一次样，给后续自检、保护和协议模块提供初始数据。 */
    ctx->raw = AdcDriver_ReadLatest();
    ctx->measurements = AdcDriver_Convert(&ctx->raw, &ctx->params);
    Protection_Init(&ctx->protection, EpwmDriver_SetTrip);
    ProtocolService_Init(&ctx->protocol, &ctx->params, tx_frame, control_handler);

    /* 上电自检失败不会立刻死机，但会记录故障并进入保护逻辑。 */
    SelfTestResult power_on = SelfTest_RunPowerOn(&ctx->raw);
    ctx->protection.selftest_failed = !SelfTest_Passed(&power_on);
    if (ctx->protection.selftest_failed)
    {
        publish_fault(ctx, SSCB_FAULT_SELFTEST);
    }

    /* 记录周期任务基准时间，后面靠时间差来调度各类任务。 */
    ctx->last_10ms = Timebase_NowMs();
    ctx->last_realtime = Timebase_NowMs();
    ctx->last_heartbeat = Timebase_NowMs();
    ctx->initialized = true;
    return SSCB_OK;
}

void System_RunOnce(SystemContext *ctx)
{
    /* 主循环每转一次就执行一次这里。 */
    if ((ctx == 0) || !ctx->initialized)
    {
        return;
    }

    uint32_t now = Timebase_NowMs();
    /* 先更新最新测量值，后面的保护和通信都依赖这些数据。 */
    ctx->raw = AdcDriver_ReadLatest();
    ctx->measurements = AdcDriver_Convert(&ctx->raw, &ctx->params);
    ctx->measurements.current_rms_a = Protection_AddCurrentSample(&ctx->protection, ctx->measurements.current_a);

    /* 轮询 CAN，处理上位机发送过来的参数/控制指令。 */
    CanDriver_Poll();

    /* 每 10ms 执行一次保护和周期自检。 */
    if ((uint32_t)(now - ctx->last_10ms) >= SSCB_CONTROL_PERIOD_MS)
    {
        ctx->last_10ms = now;
        SelfTestResult periodic = SelfTest_RunPeriodic(&ctx->measurements);
        ctx->protection.selftest_failed = !SelfTest_Passed(&periodic);
        if (ctx->protection.selftest_failed)
        {
            publish_fault(ctx, SSCB_FAULT_SELFTEST);
        }

        SscbFaultCode fault = Protection_Evaluate10ms(&ctx->protection, &ctx->params, &ctx->measurements, now);
        publish_fault(ctx, fault);
        (void)Protection_UpdateRecovery(&ctx->protection, &ctx->params, &ctx->measurements, now);
    }

    /* 按固定周期向上位机发送实时测量值。 */
    if ((uint32_t)(now - ctx->last_realtime) >= SSCB_REALTIME_PERIOD_MS)
    {
        ctx->last_realtime = now;
        ProtocolService_SendRealtime(&ctx->protocol, &ctx->measurements, ctx->protection.state, ctx->protection.fault_bits, ctx->protection.selftest_failed);
    }

    /* 心跳报文主要用于告诉上位机“设备还活着”。 */
    if ((uint32_t)(now - ctx->last_heartbeat) >= SSCB_HEARTBEAT_PERIOD_MS)
    {
        ctx->last_heartbeat = now;
        ProtocolService_SendHeartbeat(&ctx->protocol, ctx->protection.state, Timebase_NowMs() / 1000u);
    }

    /* 没有别的事可做时进入空闲态，真实芯片上通常意味着省电或等待中断。 */
    Board_Idle();
}

void System_OnCanRx(const SscbCanFrame *frame)
{
    if ((s_ctx != 0) && (frame != 0))
    {
        /* 交给协议层解析报文；参数被修改后，顺便刷新短路阈值。 */
        ProtocolService_OnRx(&s_ctx->protocol, frame);
        CmpssDriver_SetThreshold(s_ctx->params.short_threshold_a);
    }
}

void System_OnShortTripInterrupt(void)
{
    /* 这个入口预期由硬件比较器/中断触发，用于极快响应短路故障。 */
    if (s_ctx == 0)
    {
        return;
    }

    SscbFaultCode fault = Protection_OnShortTrip(&s_ctx->protection, &s_ctx->measurements, Timebase_NowMs());
    publish_fault(s_ctx, fault);
}
