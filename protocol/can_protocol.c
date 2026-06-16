#include <string.h>
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

static float u32_to_float(uint32_t raw)
{
    float value;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

static uint32_t float_to_u32(float value)
{
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    return raw;
}

static int valid_node(uint8_t node_id)
{
    return node_id >= 1u && node_id <= 127u;
}

static uint16_t make_id(uint16_t base, uint8_t node_id)
{
    return (uint16_t)(base + node_id);
}

sscb_status_t sscb_can_make_realtime(uint8_t node_id, uint16_t status,
                                     const sscb_measurements_t *m,
                                     sscb_can_frame_t *frame)
{
    if (!valid_node(node_id) || m == 0 || frame == 0) {
        return SSCB_ERR_ARG;
    }
    frame->id = make_id(SSCB_CAN_ID_REALTIME_BASE, node_id);
    frame->dlc = 8u;
    put_u16_le(&frame->data[0], m->voltage_dv);
    put_u16_le(&frame->data[2], (uint16_t)m->current_pga_da);
    put_u16_le(&frame->data[4], (uint16_t)m->temperature_dc);
    frame->data[6] = (uint8_t)(status & 0xFFu);
    frame->data[7] = (uint8_t)(status >> 8);
    return SSCB_OK;
}

sscb_status_t sscb_can_make_fault(uint8_t node_id, sscb_fault_t fault,
                                  uint32_t timestamp_ms,
                                  int16_t current_pga_da,
                                  uint16_t voltage_dv,
                                  sscb_can_frame_t *frame)
{
    if (!valid_node(node_id) || frame == 0) {
        return SSCB_ERR_ARG;
    }
    frame->id = make_id(SSCB_CAN_ID_FAULT_BASE, node_id);
    frame->dlc = 8u;
    frame->data[0] = (uint8_t)fault;
    put_u32_le(&frame->data[1], timestamp_ms);
    put_u16_le(&frame->data[5], (uint16_t)current_pga_da);
    frame->data[7] = (uint8_t)(voltage_dv / 100u);
    return SSCB_OK;
}

sscb_status_t sscb_can_make_heartbeat(uint8_t node_id, sscb_state_t state,
                                      uint64_t runtime_seconds, sscb_can_frame_t *frame)
{
    uint64_t i;

    if (!valid_node(node_id) || frame == 0) {
        return SSCB_ERR_ARG;
    }
    frame->id = make_id(SSCB_CAN_ID_HEARTBEAT_BASE, node_id);
    frame->dlc = 8u;
    frame->data[0] = node_id;
    frame->data[1] = (uint8_t)state;
    for (i = 0u; i < 6u; i++) {
        frame->data[2u + i] = (uint8_t)((runtime_seconds >> (8u * i)) & 0xFFu);
    }
    return SSCB_OK;
}

sscb_status_t sscb_can_handle_param_request(sscb_params_t *params,
                                            const sscb_can_frame_t *request,
                                            sscb_can_frame_t *response)
{
    uint8_t op;
    uint8_t id;
    uint32_t raw;
    float requested;
    uint32_t value;
    sscb_status_t rc;

    if (params == 0 || request == 0 || response == 0 || request->dlc != 8u) {
        return SSCB_ERR_ARG;
    }

    op = request->data[0];
    id = request->data[1];
    raw = get_u32_le(&request->data[2]);
    requested = u32_to_float(raw);
    response->id = request->id;
    response->dlc = 8u;
    response->data[1] = id;
    response->data[6] = 0u;
    response->data[7] = 0u;

    if (op == 0x10u) {
        rc = sscb_params_get(params, id, &value);
        if (rc == SSCB_OK) {
            raw = float_to_u32((float)value);
        }
    } else if (op == 0x11u) {
        value = (requested < 0.0f) ? 0u : (uint32_t)requested;
        rc = sscb_params_set(params, id, value);
        if (rc == SSCB_OK) {
            rc = sscb_params_get(params, id, &value);
            if (rc == SSCB_OK) {
                raw = float_to_u32((float)value);
            }
        }
    } else {
        rc = SSCB_ERR_ARG;
    }

    response->data[0] = (uint8_t)(rc == SSCB_OK ? 0u : (uint8_t)(-rc));
    put_u32_le(&response->data[2], raw);
    return rc == SSCB_OK ? SSCB_OK : rc;
}

sscb_status_t sscb_can_handle_time_sync_request(sscb_timebase_t *timebase,
                                                const sscb_can_frame_t *request,
                                                sscb_can_frame_t *response)
{
    uint32_t unix_sec;
    uint32_t millis;

    if (timebase == 0 || request == 0 || response == 0 || request->dlc != 8u) {
        return SSCB_ERR_ARG;
    }

    unix_sec = get_u32_le(&request->data[0]);
    millis = get_u32_le(&request->data[4]);
    sscb_timebase_sync(timebase, unix_sec, millis);

    response->id = request->id;
    response->dlc = 8u;
    put_u32_le(&response->data[0], unix_sec);
    put_u32_le(&response->data[4], millis);
    return SSCB_OK;
}

sscb_status_t sscb_can_handle_fault_log_request(uint8_t node_id,
                                                sscb_fault_log_t *log,
                                                const sscb_can_frame_t *request,
                                                sscb_can_frame_t *response)
{
    uint16_t index;
    uint16_t count;
    sscb_fault_record_t record;
    sscb_status_t rc;

    if (log == 0 || request == 0 || response == 0 || request->dlc != 8u) {
        return SSCB_ERR_ARG;
    }

    count = sscb_fault_log_count(log);
    index = request->data[1];
    if (request->data[0] == 0x03u) {
        if (count == 0u) {
            response->id = request->id;
            response->dlc = 8u;
            response->data[0] = 0u;
            response->data[1] = 0u;
            memset(&response->data[2], 0, 6u);
            return SSCB_OK;
        }
        if (index >= count) {
            index = (uint16_t)(count - 1u);
        }
        rc = sscb_fault_log_get_chronological(log, index, &record);
        if (rc != SSCB_OK) {
            return rc;
        }
        response->id = make_id(SSCB_CAN_ID_FAULT_BASE, node_id);
        response->dlc = 8u;
        return sscb_can_make_fault(node_id, record.fault, (uint32_t)record.timestamp_ms,
                                   record.current_pga_da, record.voltage_dv, response);
    }

    if (request->data[0] == 0x04u) {
        sscb_fault_log_clear(log);
        response->id = request->id;
        response->dlc = 8u;
        response->data[0] = 0u;
        response->data[1] = 0u;
        memset(&response->data[2], 0, 6u);
        return SSCB_OK;
    }

    return SSCB_ERR_ARG;
}
