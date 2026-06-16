#include "app/firmware.h"
#include "bsp/board.h"
#include "bsp/board_resources.h"
#include "driver/adc_driver.h"
#include "driver/can_driver.h"
#include "driver/cmpss_driver.h"
#include "driver/driver_status.h"
#include "driver/epwm_driver.h"
#include "driver/gate_driver.h"
#include "protocol/can_protocol.h"
#include "protocol/protocol_service.h"
#include "selftest/selftest.h"

static void send_periodic_frames(sscb_firmware_t *fw, const sscb_measurements_t *m)
{
    sscb_can_frame_t frame;

    fw->realtime_elapsed_ms++;
    fw->heartbeat_elapsed_ms++;

    if (fw->realtime_elapsed_ms >= fw->system.params.realtime_period_ms) {
        fw->realtime_elapsed_ms = 0u;
        if (sscb_can_make_realtime(fw->system.params.node_id, fw->system.status_word, m, &frame) == SSCB_OK) {
            (void)sscb_can_driver_send(&frame);
        }
    }

    if (fw->heartbeat_elapsed_ms >= fw->system.params.heartbeat_period_ms) {
        fw->heartbeat_elapsed_ms = 0u;
        if (sscb_can_make_heartbeat(fw->system.params.node_id, fw->system.state,
                                    fw->system.status_word, &frame) == SSCB_OK) {
            (void)sscb_can_driver_send(&frame);
        }
    }
}

static void handle_can_request(sscb_firmware_t *fw)
{
    sscb_can_frame_t request;
    sscb_can_frame_t response;

    if (sscb_can_driver_receive(&request) != SSCB_OK) {
        return;
    }

    if (sscb_protocol_handle_frame(&fw->system, &request, &response) == SSCB_OK) {
        (void)sscb_can_driver_send(&response);
        (void)sscb_param_store_save(&fw->param_store, &fw->system.params);
        (void)sscb_cmpss_driver_set_dac(sscb_short_threshold_to_dac_code(&fw->system.params));
    }
}

sscb_status_t sscb_firmware_init(sscb_firmware_t *fw)
{
    sscb_selftest_inputs_t inputs;
    sscb_selftest_result_t selftest;
    sscb_driver_status_t driver_status;

    if (fw == 0) {
        return SSCB_ERR_ARG;
    }

    sscb_system_init(&fw->system);
    sscb_timebase_init(&fw->timebase);
    sscb_fram_init(&fw->fram);
    sscb_param_store_init(&fw->param_store);
    fw->gate_driver_config = sscb_gate_driver_default_config();
    fw->realtime_elapsed_ms = 0u;
    fw->heartbeat_elapsed_ms = 0u;

    if (sscb_board_init() != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_gate_driver_init() != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_gate_driver_apply_config(&fw->gate_driver_config) != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_gate_driver_verify_config(&fw->gate_driver_config) != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_adc_driver_init() != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_epwm_driver_init_trip_chain() != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_cmpss_driver_init(sscb_short_threshold_to_dac_code(&fw->system.params)) != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_can_driver_init(SSCB_CAN_BITRATE) != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_fram_hw_init() != SSCB_OK) return SSCB_ERR_ARG;
    if (sscb_driver_status_init() != SSCB_OK) return SSCB_ERR_ARG;

    (void)sscb_param_store_load(&fw->param_store, &fw->system.params);
    (void)sscb_cmpss_driver_set_dac(sscb_short_threshold_to_dac_code(&fw->system.params));

    driver_status = sscb_driver_status_read();
    inputs.adc_range_ok = true;
    inputs.ntc_ok = true;
    inputs.cmpss_config_ok = sscb_cmpss_driver_last_dac() <= 4095u;
    inputs.driver_ready = driver_status.ready;
    inputs.driver_fault_clear = driver_status.fault_clear;
    selftest = sscb_selftest_run(&inputs);

    if (selftest.fault != SSCB_FAULT_NONE) {
        fw->system.state = SSCB_STATE_LOCKOUT;
        fw->system.status_word |= SSCB_STATUS_SELFTEST_FAIL | SSCB_STATUS_LOCKOUT;
    }

    return SSCB_OK;
}

void sscb_firmware_run_once(sscb_firmware_t *fw)
{
    sscb_adc_raw_t raw;
    sscb_measurements_t m;
    sscb_driver_status_t driver_status;

    if (fw == 0) {
        return;
    }

    raw = sscb_adc_driver_read_latest();
    m = sscb_adc_convert_measurements(&raw, &fw->system.params);
    driver_status = sscb_driver_status_read();
    m.driver_flags = driver_status.flags;
    sscb_system_set_safety_inputs(&fw->system, true, driver_status.ready, true);

    sscb_protection_sample_current_ma(&fw->system.protection, (int32_t)m.current_pga_da * 100);
    sscb_system_tick_1ms(&fw->system, &m);

    if (!driver_status.fault_clear) {
        sscb_system_on_short_trip(&fw->system, &m);
    }

    if (fw->system.state == SSCB_STATE_RUN || fw->system.state == SSCB_STATE_WARN) {
        sscb_epwm_allow_output(true);
    } else {
        sscb_epwm_force_off();
    }

    send_periodic_frames(fw, &m);
    handle_can_request(fw);
    sscb_timebase_tick_ms(&fw->timebase);
}
