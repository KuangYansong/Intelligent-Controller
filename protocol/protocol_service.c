#include "protocol_service.h"
#include "param_store.h"
#include "timebase.h"

void ProtocolService_Init(ProtocolService *svc, SscbParams *params, ProtocolFrameTx tx, ProtocolControlHandler control_handler)
{
    svc->params = params;
    svc->tx = tx;
    svc->control_handler = control_handler;
}

void ProtocolService_OnRx(ProtocolService *svc, const SscbCanFrame *frame)
{
    uint32_t unix_sec;
    uint32_t millis;
    bool write;
    SscbParamId param_id;
    float value = 0.0f;
    SscbControlCommand cmd;

    if ((svc == 0) || (svc->params == 0) || (frame == 0))
    {
        return;
    }

    if (CanProtocol_ParseTimeSync(frame, svc->params->node_id, &unix_sec, &millis))
    {
        Timebase_SyncUnix(unix_sec, millis);
        return;
    }

    if (CanProtocol_ParseControl(frame, svc->params->node_id, &cmd))
    {
        if (svc->control_handler != 0)
        {
            svc->control_handler(cmd);
        }
        return;
    }

    if (CanProtocol_ParseParamRequest(frame, svc->params->node_id, &write, &param_id, &value))
    {
        SscbStatus status;
        if (write)
        {
            status = ParamStore_Set(svc->params, param_id, value);
        }
        else
        {
            status = ParamStore_Get(svc->params, param_id, &value);
        }

        if (svc->tx != 0)
        {
            SscbCanFrame response = CanProtocol_ParamResponse(svc->params->node_id, status, param_id, value);
            svc->tx(&response);
        }
    }
}

void ProtocolService_SendRealtime(ProtocolService *svc, const SscbMeasurements *m, SscbSystemState state, uint32_t fault_bits, bool selftest_failed)
{
    if ((svc != 0) && (svc->tx != 0) && (svc->params != 0) && (m != 0))
    {
        SscbCanFrame frame = CanProtocol_RealtimeFrame(svc->params->node_id, m, state, fault_bits, selftest_failed);
        svc->tx(&frame);
    }
}

void ProtocolService_SendFault(ProtocolService *svc, const SscbFaultRecord *record)
{
    if ((svc != 0) && (svc->tx != 0) && (svc->params != 0) && (record != 0))
    {
        SscbCanFrame frame = CanProtocol_FaultFrame(svc->params->node_id, record);
        svc->tx(&frame);
    }
}

void ProtocolService_SendHeartbeat(ProtocolService *svc, SscbSystemState state, uint64_t runtime_s)
{
    if ((svc != 0) && (svc->tx != 0) && (svc->params != 0))
    {
        SscbCanFrame frame = CanProtocol_HeartbeatFrame(svc->params->node_id, state, runtime_s);
        svc->tx(&frame);
    }
}

