#include "can_driver.h"
#include "sscb_config.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#endif

static CanRxCallback s_rx_callback;
static SscbCanFrame s_last_tx;

SscbStatus CanDriver_Init(CanRxCallback callback)
{
    /* 保存回调，真实硬件或后续扩展接收逻辑时会用到。 */
    s_rx_callback = callback;
#ifdef SSCB_TARGET_C2000
    /* 硬件模式下初始化 CANB 并配置波特率。 */
    CAN_initModule(CANB_BASE);
    CAN_setBitRate(CANB_BASE, DEVICE_SYSCLK_FREQ, SSCB_CAN_BITRATE, 20u);
    CAN_startModule(CANB_BASE);
#endif
    return SSCB_OK;
}

SscbStatus CanDriver_Send(const SscbCanFrame *frame)
{
    if (frame == 0)
    {
        return SSCB_BAD_PARAM;
    }
    /* 保存最后一次发送的帧，方便主机模式调试或后续测试验证。 */
    s_last_tx = *frame;
#ifdef SSCB_TARGET_C2000
    CAN_sendMessage(CANB_BASE, 1u, frame->dlc, frame->data);
#endif
    return SSCB_OK;
}

void CanDriver_Poll(void)
{
    /* 目前没有真正的轮询接收逻辑，这里先保留接口形状。 */
    (void)s_rx_callback;
    (void)s_last_tx;
}
