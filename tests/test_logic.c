#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "can_protocol.h"
#include "crc16.h"
#include "fault_log.h"
#include "fram.h"
#include "i2t.h"
#include "param_store.h"
#include "protection.h"
#include "rms.h"
#include "timebase.h"

static void test_crc(void)
{
    const char data[] = "123456789";
    assert(Crc16_CcittFalse(data, 9u) == 0x29B1u);
}

static void test_params(void)
{
    SscbParams params;
    assert(Fram_Init() == SSCB_OK);
    ParamStore_Defaults(&params);
    assert(ParamStore_Save(&params) == SSCB_OK);
    assert(ParamStore_Load(&params) == SSCB_OK);
    assert(params.node_id == SSCB_DEFAULT_NODE_ID);
    assert(ParamStore_Set(&params, SSCB_PARAM_NODE_ID, 2.0f) == SSCB_OK);
    assert(ParamStore_Load(&params) == SSCB_OK);
    assert(params.node_id == 2u);
}

static void test_fault_log(void)
{
    assert(FaultLog_Init() == SSCB_OK);
    for (uint8_t i = 0u; i < 55u; i++)
    {
        SscbFaultRecord record;
        record.fault = SSCB_FAULT_OVERLOAD;
        record.timestamp_ms = i;
        record.voltage_v = (float)i;
        record.current_a = (float)i;
        record.temperature_c = 25.0f;
        assert(FaultLog_Append(&record) == SSCB_OK);
    }
    assert(FaultLog_Count() == SSCB_FAULT_LOG_CAPACITY);
    SscbFaultRecord first;
    assert(FaultLog_Read(0u, &first) == SSCB_OK);
    assert(first.timestamp_ms == 5u);
    assert(FaultLog_Clear() == SSCB_OK);
    assert(FaultLog_Count() == 0u);
    assert(FaultLog_Read(0u, &first) == SSCB_BAD_PARAM);
}

static void test_rms_i2t(void)
{
    RmsWindow rms;
    Rms_Init(&rms);
    for (uint16_t i = 0u; i < SSCB_RMS_WINDOW_SIZE; i++)
    {
        (void)Rms_AddSample(&rms, 10.0f);
    }
    assert(fabsf(Rms_Get(&rms) - 10.0f) < 0.01f);

    I2tState i2t;
    I2t_Init(&i2t);
    bool tripped = false;
    for (uint16_t i = 0u; i < 1000u && !tripped; i++)
    {
        tripped = I2t_Update(&i2t, 20.0f, 10.0f, 10.0f, 0.010f);
    }
    assert(tripped);
}

static void test_protocol(void)
{
    SscbMeasurements m;
    m.voltage_v = 800.0f;
    m.current_a = 12.0f;
    m.current_rms_a = 11.5f;
    m.temperature_c = 30.0f;
    SscbCanFrame frame = CanProtocol_RealtimeFrame(1u, &m, SSCB_STATE_NORMAL, 0u, false);
    assert(frame.id == 0x101u);
    assert(frame.data[0] == 0x40u);
    assert(frame.data[1] == 0x1Fu);
    assert(frame.data[6] == 0x01u);
}

int main(void)
{
    test_crc();
    test_params();
    test_fault_log();
    test_rms_i2t();
    test_protocol();
    puts("logic tests passed");
    return 0;
}
