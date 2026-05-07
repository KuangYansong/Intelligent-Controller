#include "can_protocol.h"
#include <string.h>
#include "utils.h"

uint16_t CanProtocol_MakeId(uint16_t base, uint8_t node_id)
{
    /* 设备通过“基地址 + 节点号”区分不同类型报文。 */
    return (uint16_t)(base + (uint16_t)node_id);
}

uint8_t CanProtocol_StatusByte(SscbSystemState state, uint32_t fault_bits, bool selftest_failed)
{
    /* 把多个状态压缩进 1 个字节，便于上位机快速解析。 */
    uint8_t status = 0u;
    if (state == SSCB_STATE_NORMAL)
    {
        status |= 0x01u;
    }
    if ((fault_bits & (1UL << SSCB_FAULT_SHORT)) != 0u)
    {
        status |= 0x02u;
    }
    if ((fault_bits & (1UL << SSCB_FAULT_OVERLOAD)) != 0u)
    {
        status |= 0x04u;
    }
    if ((fault_bits & (1UL << SSCB_FAULT_OVERVOLTAGE)) != 0u)
    {
        status |= 0x08u;
    }
    if ((fault_bits & (1UL << SSCB_FAULT_OVERTEMP)) != 0u)
    {
        status |= 0x10u;
    }
    if (state == SSCB_STATE_LOCKOUT)
    {
        status |= 0x20u;
    }
    if (selftest_failed)
    {
        status |= 0x40u;
    }
    return status;
}

SscbCanFrame CanProtocol_RealtimeFrame(uint8_t node_id, const SscbMeasurements *m, SscbSystemState state, uint32_t fault_bits, bool selftest_failed)
{
    SscbCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = CanProtocol_MakeId(SSCB_CAN_ID_REALTIME_BASE, node_id);
    frame.dlc = 8u;

    /* 实时帧把电压、电流、温度和状态一起压缩到 8 字节里。 */
    Sscb_PutLe16(&frame.data[0], Sscb_ClampU16(m->voltage_v, 10.0f));
    Sscb_PutLe16(&frame.data[2], Sscb_ClampU16(m->current_rms_a, 10.0f));
    Sscb_PutLe16(&frame.data[4], (uint16_t)Sscb_ClampI16(m->temperature_c, 10.0f));
    frame.data[6] = CanProtocol_StatusByte(state, fault_bits, selftest_failed);
    frame.data[7] = 0u;
    return frame;
}

SscbCanFrame CanProtocol_FaultFrame(uint8_t node_id, const SscbFaultRecord *record)
{
    SscbCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = CanProtocol_MakeId(SSCB_CAN_ID_FAULT_BASE, node_id);
    frame.dlc = 8u;
    /* 故障帧携带故障类型、时间戳和故障时的关键测量值。 */
    frame.data[0] = (uint8_t)record->fault;
    Sscb_PutLe32(&frame.data[1], record->timestamp_ms);
    Sscb_PutLe16(&frame.data[5], Sscb_ClampU16(record->current_a, 10.0f));
    frame.data[7] = (uint8_t)Sscb_ClampU16(record->voltage_v, 0.1f);
    return frame;
}

SscbCanFrame CanProtocol_HeartbeatFrame(uint8_t node_id, SscbSystemState state, uint64_t runtime_s)
{
    SscbCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = CanProtocol_MakeId(SSCB_CAN_ID_HEARTBEAT_BASE, node_id);
    frame.dlc = 8u;
    /* 心跳帧用于表明节点在线，并附带累计运行秒数。 */
    frame.data[0] = node_id;
    frame.data[1] = (uint8_t)state;
    for (uint8_t i = 0u; i < 6u; i++)
    {
        frame.data[2u + i] = (uint8_t)((runtime_s >> (8u * i)) & 0xFFu);
    }
    return frame;
}

SscbCanFrame CanProtocol_ParamResponse(uint8_t node_id, SscbStatus status, SscbParamId id, float value)
{
    SscbCanFrame frame;
    uint32_t raw;
    memset(&frame, 0, sizeof(frame));
    frame.id = CanProtocol_MakeId(SSCB_CAN_ID_PARAM_BASE, node_id);
    frame.dlc = 8u;
    /* 参数响应里把 float 原样按 4 字节拷贝，避免文本化带来的额外开销。 */
    frame.data[0] = (uint8_t)((status == SSCB_OK) ? 0u : 1u);
    frame.data[1] = (uint8_t)id;
    memcpy(&raw, &value, sizeof(raw));
    Sscb_PutLe32(&frame.data[2], raw);
    return frame;
}

bool CanProtocol_IsForNode(const SscbCanFrame *frame, uint16_t base, uint8_t node_id)
{
    /* 先按报文类型基地址判断，再看是不是发给当前节点。 */
    return (frame != 0) && (frame->id == CanProtocol_MakeId(base, node_id));
}

bool CanProtocol_ParseTimeSync(const SscbCanFrame *frame, uint8_t node_id, uint32_t *unix_sec, uint32_t *millis)
{
    if (!CanProtocol_IsForNode(frame, SSCB_CAN_ID_TIME_BASE, node_id) || (frame->dlc < 8u))
    {
        return false;
    }
    /* 时间同步帧前 4 字节是秒，后 4 字节是当前秒内的毫秒值。 */
    *unix_sec = Sscb_GetLe32(&frame->data[0]);
    *millis = Sscb_GetLe32(&frame->data[4]);
    return true;
}

bool CanProtocol_ParseControl(const SscbCanFrame *frame, uint8_t node_id, SscbControlCommand *cmd)
{
    if (!CanProtocol_IsForNode(frame, SSCB_CAN_ID_CONTROL_BASE, node_id) || (frame->dlc < 1u) || (cmd == 0))
    {
        return false;
    }
    *cmd = (SscbControlCommand)frame->data[0];
    return true;
}

bool CanProtocol_ParseParamRequest(const SscbCanFrame *frame, uint8_t node_id, bool *write, SscbParamId *id, float *value)
{
    if (!CanProtocol_IsForNode(frame, SSCB_CAN_ID_PARAM_BASE, node_id) ||
        (frame->dlc < 8u) ||
        (write == 0) ||
        (id == 0) ||
        (value == 0))
    {
        return false;
    }

    if (frame->data[0] == 0x10u)
    {
        /* 0x10 表示读参数。 */
        *write = false;
    }
    else if (frame->data[0] == 0x11u)
    {
        /* 0x11 表示写参数。 */
        *write = true;
    }
    else
    {
        return false;
    }

    *id = (SscbParamId)frame->data[1];
    /* 请求帧中的参数值同样按原始 float 位模式传输。 */
    uint32_t raw = Sscb_GetLe32(&frame->data[2]);
    memcpy(value, &raw, sizeof(raw));
    return true;
}
