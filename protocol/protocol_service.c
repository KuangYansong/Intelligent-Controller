#include "protocol/protocol_service.h"
#include "protocol/can_protocol.h"

sscb_status_t sscb_protocol_handle_frame(sscb_system_t *sys,
                                         const sscb_can_frame_t *request,
                                         sscb_can_frame_t *response)
{
    if (sys == 0 || request == 0 || response == 0) {
        return SSCB_ERR_ARG;
    }

    if (request->id == SSCB_CAN_ID_PARAM_FIXED) {
        return sscb_can_handle_param_request(&sys->params, request, response);
    }

    if (request->id == SSCB_CAN_ID_CONTROL_FIXED && request->dlc == 8u) {
        if (request->data[0] == 0x01u) {
            return sscb_system_manual_reset(sys);
        }
        return SSCB_ERR_NOT_FOUND;
    }

    return SSCB_ERR_NOT_FOUND;
}
