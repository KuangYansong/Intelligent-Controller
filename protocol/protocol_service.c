#include "protocol/protocol_service.h"
#include "protocol/can_protocol.h"

sscb_status_t sscb_protocol_handle_frame(sscb_system_t *sys,
                                         const sscb_can_frame_t *request,
                                         sscb_can_frame_t *response)
{
    uint8_t node_id;
    uint16_t param_id;
    uint16_t time_id;
    uint16_t control_id;

    if (sys == 0 || request == 0 || response == 0) {
        return SSCB_ERR_ARG;
    }

    node_id = sys->params.node_id;
    param_id = (uint16_t)(SSCB_CAN_ID_PARAM_BASE + node_id);
    time_id = (uint16_t)(SSCB_CAN_ID_TIME_SYNC_BASE + node_id);
    control_id = (uint16_t)(SSCB_CAN_ID_CONTROL_BASE + node_id);

    if (request->id == param_id) {
        return sscb_can_handle_param_request(&sys->params, request, response);
    }

    if (request->id == time_id) {
        return sscb_can_handle_time_sync_request(sys->timebase, request, response);
    }

    if (request->id == control_id && request->dlc == 8u) {
        if (request->data[0] == 0x01u) {
            response->id = request->id;
            response->dlc = 8u;
            response->data[0] = (uint8_t)(sscb_system_manual_reset(sys) == SSCB_OK ? 0u : 1u);
            response->data[1] = 0u;
            response->data[2] = 0u;
            response->data[3] = 0u;
            response->data[4] = 0u;
            response->data[5] = 0u;
            response->data[6] = 0u;
            response->data[7] = 0u;
            return SSCB_OK;
        }
        if (request->data[0] == 0x03u || request->data[0] == 0x04u) {
            return sscb_can_handle_fault_log_request(node_id, &sys->fault_log, request, response);
        }
        if (request->data[0] == 0x02u) {
            response->id = request->id;
            response->dlc = 8u;
            response->data[0] = 0u;
            return SSCB_OK;
        }
        return SSCB_ERR_NOT_FOUND;
    }

    return SSCB_ERR_NOT_FOUND;
}
