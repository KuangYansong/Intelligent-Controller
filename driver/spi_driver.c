#include "spi_driver.h"

#ifdef SSCB_TARGET_C2000
#include "driverlib.h"
#include "device.h"
#include "pin_map.h"
#endif

#define FRAM_CMD_WREN           (0x06u)
#define FRAM_CMD_READ           (0x03u)
#define FRAM_CMD_WRITE          (0x02u)

#define FRAM_SPI_CHAR_LEN       (8u)
#define FRAM_CS_GPIO            (15u)

#ifdef SSCB_TARGET_C2000
static void fram_select(void)
{
    GPIO_writePin(FRAM_CS_GPIO, 0u);
}

static void fram_deselect(void)
{
    GPIO_writePin(FRAM_CS_GPIO, 1u);
}

static uint8_t spi_transfer_byte(uint8_t tx)
{
    return (uint8_t)SPI_pollingNonFIFOTransaction(SPIB_BASE, FRAM_SPI_CHAR_LEN, tx);
}

static void fram_send_address(uint32_t address)
{
    spi_transfer_byte((uint8_t)((address >> 16u) & 0xFFu));
    spi_transfer_byte((uint8_t)((address >> 8u) & 0xFFu));
    spi_transfer_byte((uint8_t)(address & 0xFFu));
}

static void fram_write_enable(void)
{
    fram_select();
    spi_transfer_byte(FRAM_CMD_WREN);
    fram_deselect();
}
#endif

SscbStatus SpiDriver_Init(void)
{
#ifdef SSCB_TARGET_C2000
    GPIO_setPinConfig(GPIO_6_SPISOMIB);
    GPIO_setPinConfig(GPIO_7_SPISIMOB);
    GPIO_setPinConfig(GPIO_14_SPICLKB);
    GPIO_setDirectionMode(6u, GPIO_DIR_MODE_IN);
    GPIO_setDirectionMode(7u, GPIO_DIR_MODE_OUT);
    GPIO_setDirectionMode(14u, GPIO_DIR_MODE_OUT);

    /* 片选脚用普通 GPIO 手动控制，避免依赖外设自动片选时序。 */
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(FRAM_CS_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_writePin(FRAM_CS_GPIO, 1u);

    /* SPIB 配成主机模式，时钟 10MHz，8bit 数据帧。 */
    SPI_disableModule(SPIB_BASE);
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA0, SPI_MODE_MASTER, 10000000u, 8u);
    SPI_disableFIFO(SPIB_BASE);
    SPI_disableLoopback(SPIB_BASE);
    SPI_setEmulationMode(SPIB_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableModule(SPIB_BASE);
#endif
    return SSCB_OK;
}

SscbStatus SpiDriver_FramRead(uint32_t address, uint8_t *data, size_t len)
{
    if ((data == 0) || (len == 0u))
    {
        return SSCB_BAD_PARAM;
    }

#ifdef SSCB_TARGET_C2000
    fram_select();
    spi_transfer_byte(FRAM_CMD_READ);
    fram_send_address(address);
    for (size_t i = 0u; i < len; i++)
    {
        data[i] = spi_transfer_byte(0xFFu);
    }
    fram_deselect();
    return SSCB_OK;
#else
    (void)address;
    (void)data;
    (void)len;
    return SSCB_NOT_READY;
#endif
}

SscbStatus SpiDriver_FramWrite(uint32_t address, const uint8_t *data, size_t len)
{
    if ((data == 0) || (len == 0u))
    {
        return SSCB_BAD_PARAM;
    }

#ifdef SSCB_TARGET_C2000
    fram_write_enable();

    fram_select();
    spi_transfer_byte(FRAM_CMD_WRITE);
    fram_send_address(address);
    for (size_t i = 0u; i < len; i++)
    {
        spi_transfer_byte(data[i]);
    }
    fram_deselect();
    return SSCB_OK;
#else
    (void)address;
    (void)data;
    (void)len;
    return SSCB_NOT_READY;
#endif
}
