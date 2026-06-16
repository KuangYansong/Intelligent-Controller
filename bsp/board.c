#include "bsp/board.h"
#include "bsp/board_resources.h"

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

sscb_status_t sscb_board_init(void)
{
#ifdef __TMS320C28XX__
    Device_init();
    Device_initGPIO();

    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setPinConfig(GPIO_16_CANB_TX);
    GPIO_setPinConfig(GPIO_17_CANB_RX);
    GPIO_setPinConfig(GPIO_6_SPIB_SOMI);
    GPIO_setPinConfig(GPIO_7_SPIB_SIMO);
    GPIO_setPinConfig(GPIO_14_SPIB_CLK);
    GPIO_setPinConfig(GPIO_15_SPIB_STE);
    GPIO_setPinConfig(GPIO_0_I2CA_SDA);
    GPIO_setPinConfig(GPIO_1_I2CA_SCL);

    GPIO_setPadConfig(SSCB_GPIO_FLT_N, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(SSCB_GPIO_FLT_N, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(SSCB_GPIO_FLT_N, GPIO_QUAL_3SAMPLE);

    GPIO_setPadConfig(SSCB_GPIO_RDYC, GPIO_PIN_TYPE_PULLUP);
    GPIO_setDirectionMode(SSCB_GPIO_RDYC, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(SSCB_GPIO_RDYC, GPIO_QUAL_3SAMPLE);

    Interrupt_initModule();
    Interrupt_initVectorTable();
#endif
    return SSCB_OK;
}

void sscb_board_enter_idle(void)
{
#ifdef __TMS320C28XX__
    IDLE;
#endif
}
