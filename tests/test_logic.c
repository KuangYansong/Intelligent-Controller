#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "app/system.h"
#include "can_protocol.h"
#include "crc16.h"
#include "fault_log.h"
#include "fram.h"
#include "protocol/protocol_service.h"
#include "i2t.h"
#include "param_store.h"
#include "protection.h"
#include "rms.h"
#include "timebase.h"

static uint32_t as_u32(float value)
{
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    return raw;
}

static float as_float(uint32_t raw)
{
    float value;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

static uint32_t from_bytes32(const uint8_t *data)
{
    uint32_t raw;
    memcpy(&raw, data, sizeof(raw));
    return raw;
}

static void put_u32_le(uint8_t *dst, uint32_t v)
{
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8) & 0xFFu);
    dst[2] = (uint8_t)((v >> 16) & 0xFFu);
    dst[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void test_crc(void)
{
    const char data[] = "123456789";
    assert(sscb_crc16_ccitt((const uint8_t *)data, 9u) == 0x29B1u);
}

static void test_timebase(void)
{
    sscb_timebase_t tb;

    sscb_timebase_init(&tb);
    for (uint32_t i = 0u; i < 1234u; i++) {
        sscb_timebase_tick_ms(&tb);
    }
    sscb_timebase_sync(&tb, 1000u, 234u);
    assert(sscb_timebase_absolute_ms(&tb) == 1000234u);
}

static void test_params_and_fram(void)
{
    sscb_fram_t fram;
    sscb_param_store_t store;
    sscb_params_t params;

    sscb_fram_init(&fram);
    sscb_param_store_init(&store);
    sscb_params_load_defaults(&params);
    params.node_id = 7u;
    assert(sscb_param_store_save_to_fram(&fram, &store, &params) == SSCB_OK);

    params.node_id = 1u;
    assert(sscb_param_store_load_from_fram(&fram, &store, &params) == SSCB_OK);
    assert(params.node_id == 7u);
}

static void test_fault_log_and_fram(void)
{
    sscb_fram_t fram;
    sscb_fault_log_t log;
    sscb_fault_record_t record;
    sscb_fault_record_t out;

    sscb_fram_init(&fram);
    sscb_fault_log_init(&log);

    memset(&record, 0, sizeof(record));
    record.fault = SSCB_FAULT_SHORT;
    record.state = SSCB_STATE_LOCKOUT;
    record.timestamp_ms = 1001u;
    record.voltage_dv = 8000u;
    record.current_pga_da = 123;
    assert(sscb_fault_log_append(&log, &record) == SSCB_OK);
    record.fault = SSCB_FAULT_OVERLOAD;
    record.timestamp_ms = 1002u;
    record.current_pga_da = 321;
    assert(sscb_fault_log_append(&log, &record) == SSCB_OK);

    assert(sscb_fault_log_save_to_fram(&fram, &log) == SSCB_OK);
    sscb_fault_log_clear(&log);
    assert(sscb_fault_log_count(&log) == 0u);
    assert(sscb_fault_log_load_from_fram(&fram, &log) == SSCB_OK);
    assert(sscb_fault_log_count(&log) == 2u);
    assert(sscb_fault_log_get_chronological(&log, 0u, &out) == SSCB_OK);
    assert(out.fault == SSCB_FAULT_SHORT);
    assert(sscb_fault_log_get_latest(&log, 0u, &out) == SSCB_OK);
    assert(out.fault == SSCB_FAULT_OVERLOAD);
}

static void test_protocol_frames(void)
{
    sscb_measurements_t m = {0};
    sscb_can_frame_t frame;
    sscb_can_frame_t request;
    sscb_can_frame_t response;
    sscb_params_t params;
    sscb_timebase_t tb;
    sscb_system_t sys;
    sscb_fault_record_t record;
    const uint8_t ts[] = {0x78u, 0x56u, 0x34u, 0x12u};

    m.voltage_dv = 8000u;
    m.current_pga_da = 115;
    m.temperature_dc = 300;
    assert(sscb_can_make_realtime(1u, SSCB_STATUS_NORMAL, &m, &frame) == SSCB_OK);
    assert(frame.id == 0x101u);
    assert(frame.data[0] == 0x40u);
    assert(frame.data[1] == 0x1Fu);
    assert(frame.data[6] == 0x01u);

    assert(sscb_can_make_heartbeat(1u, SSCB_STATE_LOCKOUT, 42u, &frame) == SSCB_OK);
    assert(frame.id == 0x301u);
    assert(frame.data[0] == 1u);
    assert(frame.data[1] == SSCB_STATE_LOCKOUT);
    assert(frame.data[2] == 42u);

    assert(sscb_can_make_fault(1u, SSCB_FAULT_OVERLOAD, 0x12345678u, 20, 8000u, &frame) == SSCB_OK);
    assert(frame.id == 0x201u);
    assert(frame.data[0] == SSCB_FAULT_OVERLOAD);
    assert(memcmp(&frame.data[1], ts, sizeof(ts)) == 0);
    assert(frame.data[5] == 20u);
    assert(frame.data[7] == 80u);

    sscb_params_load_defaults(&params);
    request.id = 0x401u;
    request.dlc = 8u;
    request.data[0] = 0x11u;
    request.data[1] = SSCB_PARAM_NODE_ID;
    put_u32_le(&request.data[2], as_u32(7.0f));
    assert(sscb_can_handle_param_request(&params, &request, &response) == SSCB_OK);
    assert(params.node_id == 7u);
    assert(response.data[0] == 0u);
    assert(fabsf(as_float(from_bytes32(&response.data[2])) - 7.0f) < 0.001f);

    request.data[0] = 0x10u;
    assert(sscb_can_handle_param_request(&params, &request, &response) == SSCB_OK);
    assert(fabsf(as_float(from_bytes32(&response.data[2])) - 7.0f) < 0.001f);

    sscb_timebase_init(&tb);
    request.id = 0x501u;
    request.data[0] = 0x34u;
    put_u32_le(&request.data[0], 1000u);
    put_u32_le(&request.data[4], 250u);
    assert(sscb_can_handle_time_sync_request(&tb, &request, &response) == SSCB_OK);
    assert(sscb_timebase_absolute_ms(&tb) == 1000250u);

    sscb_system_init(&sys);
    sscb_system_attach_timebase(&sys, &tb);
    sscb_fault_log_init(&sys.fault_log);
    memset(&record, 0, sizeof(record));
    record.fault = SSCB_FAULT_SHORT;
    record.state = SSCB_STATE_LOCKOUT;
    record.timestamp_ms = 123456u;
    record.current_pga_da = 22;
    record.voltage_dv = 8200u;
    assert(sscb_fault_log_append(&sys.fault_log, &record) == SSCB_OK);

    request.id = 0x601u;
    request.dlc = 8u;
    request.data[0] = 0x03u;
    request.data[1] = 0u;
    assert(sscb_protocol_handle_frame(&sys, &request, &response) == SSCB_OK);
    assert(response.id == 0x201u);
    assert(response.data[0] == SSCB_FAULT_SHORT);
    assert(response.data[7] == 82u);

    request.data[0] = 0x04u;
    assert(sscb_protocol_handle_frame(&sys, &request, &response) == SSCB_OK);
    assert(sscb_fault_log_count(&sys.fault_log) == 0u);
}

static void test_rms_i2t(void)
{
    sscb_protection_t prot;
    sscb_params_t params;
    sscb_measurements_t m = {0};
    sscb_fault_t fault;

    sscb_params_load_defaults(&params);
    sscb_protection_init(&prot, &params);
    for (uint16_t i = 0u; i < SSCB_RMS_WINDOW_SAMPLES; i++)
    {
        sscb_protection_sample_current_ma(&prot, 10000);
    }
    assert(sscb_protection_rms_a(&prot) == 10u);

    m.voltage_dv = 8000u;
    m.temperature_dc = 300;
    fault = sscb_protection_tick_1ms(&prot, &m);
    assert(fault == SSCB_FAULT_NONE);
}

int main(void)
{
    test_crc();
    test_timebase();
    test_params_and_fram();
    test_fault_log_and_fram();
    test_protocol_frames();
    test_rms_i2t();
    puts("logic tests passed");
    return 0;
}
