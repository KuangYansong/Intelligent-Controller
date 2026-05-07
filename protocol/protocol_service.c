#include "protocol_service.h"
#include "param_store.h"
#include "timebase.h"

void ProtocolService_Init(ProtocolService *svc, SscbParams *params, ProtocolFrameTx tx, ProtocolControlHandler control_handler)
{
    /* 协议服务本身很轻，只保存参数指针和两个回调。 */
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

    /* 先判断是否为时间同步帧。 */
    if (CanProtocol_ParseTimeSync(frame, svc->params->node_id, &unix_sec, &millis))
    {
        Timebase_SyncUnix(unix_sec, millis);
        return;
    }

    /* 再判断是否为控制命令，例如清故障、软件复位。 */
    if (CanProtocol_ParseControl(frame, svc->params->node_id, &cmd))
    {
        if (svc->control_handler != 0)
        {
            svc->control_handler(cmd);
        }
        return;
    }

    /* 最后处理参数读写命令，并把处理结果回发给上位机。 */
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
        /* 把实时测量值打包成一帧 CAN 报文发送出去。 */
        SscbCanFrame frame = CanProtocol_RealtimeFrame(svc->params->node_id, m, state, fault_bits, selftest_failed);
        svc->tx(&frame);
    }
}

void ProtocolService_SendFault(ProtocolService *svc, const SscbFaultRecord *record)
{
    if ((svc != 0) && (svc->tx != 0) && (svc->params != 0) && (record != 0))
    {
        /* 故障记录单独使用故障报文格式发送。 */
        SscbCanFrame frame = CanProtocol_FaultFrame(svc->params->node_id, record);
        svc->tx(&frame);
    }
}

void ProtocolService_SendHeartbeat(ProtocolService *svc, SscbSystemState state, uint64_t runtime_s)
{
    if ((svc != 0) && (svc->tx != 0) && (svc->params != 0))
    {
        /* 心跳帧只携带最基础的“我还在线”信息。 */
        SscbCanFrame frame = CanProtocol_HeartbeatFrame(svc->params->node_id, state, runtime_s);
        svc->tx(&frame);
    }
}
