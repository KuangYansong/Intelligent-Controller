#include "driver/can_driver.h"
#include "bsp/board_resources.h"

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

static uint32_t g_can_bitrate;
#ifndef __TMS320C28XX__
static sscb_can_frame_t g_last_tx;
#endif

sscb_status_t sscb_can_driver_init(uint32_t bitrate)
{
    g_can_bitrate = bitrate;
#ifdef __TMS320C28XX__
    if (bitrate == 0u) {
        return SSCB_ERR_ARG;
    }

    CAN_initModule(CANB_BASE);
    CAN_setBitRate(CANB_BASE, DEVICE_SYSCLK_FREQ, bitrate, 20u);
    CAN_setupMessageObject(CANB_BASE, SSCB_CAN_TX_OBJECT, 0u,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_TX,
                           0u, 0u, 8u);
    CAN_setupMessageObject(CANB_BASE, SSCB_CAN_RX_PARAM_OBJECT,
                           SSCB_CAN_ID_PARAM_BASE + SSCB_CAN_NODE_ID_DEFAULT,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_RX,
                           0x7FFu, CAN_MSG_OBJ_USE_ID_FILTER, 8u);
    CAN_setupMessageObject(CANB_BASE, SSCB_CAN_RX_TIME_OBJECT,
                           SSCB_CAN_ID_TIME_SYNC_BASE + SSCB_CAN_NODE_ID_DEFAULT,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_RX,
                           0x7FFu, CAN_MSG_OBJ_USE_ID_FILTER, 8u);
    CAN_setupMessageObject(CANB_BASE, SSCB_CAN_RX_CONTROL_OBJECT,
                           SSCB_CAN_ID_CONTROL_BASE + SSCB_CAN_NODE_ID_DEFAULT,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_RX,
                           0x7FFu, CAN_MSG_OBJ_USE_ID_FILTER, 8u);
    CAN_startModule(CANB_BASE);
#endif
    return g_can_bitrate == 0u ? SSCB_ERR_ARG : SSCB_OK;
}

sscb_status_t sscb_can_driver_send(const sscb_can_frame_t *frame)
{
    uint16_t data_words[4];
    if (frame == 0 || frame->dlc > 8u || g_can_bitrate == 0u) {
        return SSCB_ERR_ARG;
    }
#ifdef __TMS320C28XX__
    data_words[0] = (uint16_t)frame->data[0] | ((uint16_t)frame->data[1] << 8);
    data_words[1] = (uint16_t)frame->data[2] | ((uint16_t)frame->data[3] << 8);
    data_words[2] = (uint16_t)frame->data[4] | ((uint16_t)frame->data[5] << 8);
    data_words[3] = (uint16_t)frame->data[6] | ((uint16_t)frame->data[7] << 8);
    CAN_setupMessageObject(CANB_BASE, SSCB_CAN_TX_OBJECT, frame->id,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_TX,
                           0u, 0u, frame->dlc);
    CAN_sendMessage(CANB_BASE, SSCB_CAN_TX_OBJECT, frame->dlc, data_words);
#else
    g_last_tx = *frame;
    (void)data_words;
#endif
    return SSCB_OK;
}

sscb_status_t sscb_can_driver_receive(sscb_can_frame_t *frame)
{
    if (frame == 0 || g_can_bitrate == 0u) {
        return SSCB_ERR_ARG;
    }
#ifdef __TMS320C28XX__
    {
        uint16_t data_words[4] = {0u, 0u, 0u, 0u};
        CAN_MsgFrameType frame_type;
        uint32_t msg_id;
        bool ok;

        ok = CAN_readMessageWithID(CANB_BASE, SSCB_CAN_RX_PARAM_OBJECT,
                                   &frame_type, &msg_id, data_words);
        if (!ok) {
            ok = CAN_readMessageWithID(CANB_BASE, SSCB_CAN_RX_TIME_OBJECT,
                                       &frame_type, &msg_id, data_words);
        }
        if (!ok) {
            ok = CAN_readMessageWithID(CANB_BASE, SSCB_CAN_RX_CONTROL_OBJECT,
                                       &frame_type, &msg_id, data_words);
        }
        if (!ok) {
            return SSCB_ERR_NOT_FOUND;
        }

        frame->id = (uint16_t)msg_id;
        frame->dlc = 8u;
        frame->data[0] = (uint8_t)(data_words[0] & 0xFFu);
        frame->data[1] = (uint8_t)(data_words[0] >> 8);
        frame->data[2] = (uint8_t)(data_words[1] & 0xFFu);
        frame->data[3] = (uint8_t)(data_words[1] >> 8);
        frame->data[4] = (uint8_t)(data_words[2] & 0xFFu);
        frame->data[5] = (uint8_t)(data_words[2] >> 8);
        frame->data[6] = (uint8_t)(data_words[3] & 0xFFu);
        frame->data[7] = (uint8_t)(data_words[3] >> 8);
        return frame_type == CAN_MSG_FRAME_STD ? SSCB_OK : SSCB_ERR_ARG;
    }
#else
    *frame = g_last_tx;
    return SSCB_ERR_NOT_FOUND;
#endif
}
