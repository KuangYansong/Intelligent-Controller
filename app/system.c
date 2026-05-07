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
    (void)CanDriver_Send(frame);
}

static SscbFaultRecord make_fault_record(SscbFaultCode fault, const SscbMeasurements *m)
{
    SscbFaultRecord record;
    record.fault = fault;
    record.timestamp_ms = (uint32_t)Timebase_NowUnixMs();
    record.voltage_v = m->voltage_v;
    record.current_a = m->current_a;
    record.temperature_c = m->temperature_c;
    return record;
}

static void publish_fault(SystemContext *ctx, SscbFaultCode fault)
{
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

    s_ctx = ctx;
    Timebase_Init();

    if (Board_Init() != SSCB_OK)
    {
        return SSCB_ERROR;
    }
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

    ctx->raw = AdcDriver_ReadLatest();
    ctx->measurements = AdcDriver_Convert(&ctx->raw, &ctx->params);
    Protection_Init(&ctx->protection, EpwmDriver_SetTrip);
    ProtocolService_Init(&ctx->protocol, &ctx->params, tx_frame, control_handler);

    SelfTestResult power_on = SelfTest_RunPowerOn(&ctx->raw);
    ctx->protection.selftest_failed = !SelfTest_Passed(&power_on);
    if (ctx->protection.selftest_failed)
    {
        publish_fault(ctx, SSCB_FAULT_SELFTEST);
    }

    ctx->last_10ms = Timebase_NowMs();
    ctx->last_realtime = Timebase_NowMs();
    ctx->last_heartbeat = Timebase_NowMs();
    ctx->initialized = true;
    return SSCB_OK;
}

void System_RunOnce(SystemContext *ctx)
{
    if ((ctx == 0) || !ctx->initialized)
    {
        return;
    }

    uint32_t now = Timebase_NowMs();
    ctx->raw = AdcDriver_ReadLatest();
    ctx->measurements = AdcDriver_Convert(&ctx->raw, &ctx->params);
    ctx->measurements.current_rms_a = Protection_AddCurrentSample(&ctx->protection, ctx->measurements.current_a);

    CanDriver_Poll();

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

    if ((uint32_t)(now - ctx->last_realtime) >= SSCB_REALTIME_PERIOD_MS)
    {
        ctx->last_realtime = now;
        ProtocolService_SendRealtime(&ctx->protocol, &ctx->measurements, ctx->protection.state, ctx->protection.fault_bits, ctx->protection.selftest_failed);
    }

    if ((uint32_t)(now - ctx->last_heartbeat) >= SSCB_HEARTBEAT_PERIOD_MS)
    {
        ctx->last_heartbeat = now;
        ProtocolService_SendHeartbeat(&ctx->protocol, ctx->protection.state, Timebase_NowMs() / 1000u);
    }

    Board_Idle();
}

void System_OnCanRx(const SscbCanFrame *frame)
{
    if ((s_ctx != 0) && (frame != 0))
    {
        ProtocolService_OnRx(&s_ctx->protocol, frame);
        CmpssDriver_SetThreshold(s_ctx->params.short_threshold_a);
    }
}

void System_OnShortTripInterrupt(void)
{
    if (s_ctx == 0)
    {
        return;
    }

    SscbFaultCode fault = Protection_OnShortTrip(&s_ctx->protection, &s_ctx->measurements, Timebase_NowMs());
    publish_fault(s_ctx, fault);
}
