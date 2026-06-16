#include "protocol/can_protocol.h"

static void put_u16_le(uint8_t *dst, uint16_t v)
{
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void put_u32_le(uint8_t *dst, uint32_t v)
{
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8) & 0xFFu);
    dst[2] = (uint8_t)((v >> 16) & 0xFFu);
    dst[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint32_t get_u32_le(const uint8_t *src)
{
    return (uint32_t)src[0] |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

sscb_status_t sscb_can_make_realtime(uint8_t node_id, uint16_t status,
                                     const sscb_measurements_t *m,
                                     sscb_can_frame_t *frame)
{
    if (node_id != SSCB_CAN_FIXED_NODE_ID || m == 0 || frame == 0) {
        return SSCB_ERR_ARG;
    }
    frame->id = SSCB_CAN_ID_REALTIME_FIXED;
    frame->dlc = 8u;
    put_u16_le(&frame->data[0], m->voltage_dv);
    put_u16_le(&frame->data[2], (uint16_t)m->current_pga_da);
    put_u16_le(&frame->data[4], (uint16_t)m->temperature_dc);
    put_u16_le(&frame->data[6], status);
    return SSCB_OK;
}

sscb_status_t sscb_can_make_fault(uint8_t node_id, sscb_fault_t fault,
                                  sscb_state_t state, int16_t value_main,
                                  uint32_t timestamp_low_ms,
                                  sscb_can_frame_t *frame)
{
    if (node_id != SSCB_CAN_FIXED_NODE_ID || frame == 0) {
        return SSCB_ERR_ARG;
    }
    frame->id = SSCB_CAN_ID_FAULT_FIXED;
    frame->dlc = 8u;
    frame->data[0] = (uint8_t)fault;
    frame->data[1] = (uint8_t)state;
    put_u16_le(&frame->data[2], (uint16_t)value_main);
    put_u32_le(&frame->data[4], timestamp_low_ms);
    return SSCB_OK;
}

sscb_status_t sscb_can_make_heartbeat(uint8_t node_id, sscb_state_t state,
                                      uint16_t status, sscb_can_frame_t *frame)
{
    if (node_id != SSCB_CAN_FIXED_NODE_ID || frame == 0) {
        return SSCB_ERR_ARG;
    }
    frame->id = SSCB_CAN_ID_HEARTBEAT_FIXED;
    frame->dlc = 8u;
    frame->data[0] = (uint8_t)state;
    frame->data[1] = (uint8_t)(status & 0xFFu);
    frame->data[2] = (uint8_t)((status >> 8) & 0xFFu);
    frame->data[3] = 0u;
    frame->data[4] = 0u;
    frame->data[5] = 0u;
    frame->data[6] = 0u;
    frame->data[7] = 0u;
    return SSCB_OK;
}

sscb_status_t sscb_can_handle_param_request(sscb_params_t *params,
                                            const sscb_can_frame_t *request,
                                            sscb_can_frame_t *response)
{
    uint8_t op;
    uint8_t id;
    uint32_t value;
    sscb_status_t rc;

    if (params == 0 || request == 0 || response == 0 || request->dlc != 8u) {
        return SSCB_ERR_ARG;
    }

    op = request->data[0];
    id = request->data[1];
    value = get_u32_le(&request->data[2]);
    response->id = request->id;
    response->dlc = 8u;
    response->data[1] = id;
    response->data[6] = 0u;
    response->data[7] = 0u;

    if (op == 0x10u) {
        rc = sscb_params_get(params, id, &value);
    } else if (op == 0x11u) {
        rc = sscb_params_set(params, id, value);
        if (rc == SSCB_OK) {
            rc = sscb_params_get(params, id, &value);
        }
    } else {
        rc = SSCB_ERR_ARG;
    }

    response->data[0] = (uint8_t)(rc == SSCB_OK ? 0u : (uint8_t)(-rc));
    put_u32_le(&response->data[2], value);
    return rc == SSCB_OK ? SSCB_OK : rc;
}
