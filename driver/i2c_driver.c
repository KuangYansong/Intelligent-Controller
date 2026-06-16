#include "driver/i2c_driver.h"
#include "bsp/board_resources.h"

#ifdef __TMS320C28XX__
#include "device.h"
#include "driverlib.h"
#endif

#define SSCB_I2C_TIMEOUT_LOOPS 50000u

#ifndef __TMS320C28XX__
static uint8_t simulated_registers[128u][64u];
static uint8_t simulated_write_count;

sscb_status_t sscb_i2c_driver_init(uint32_t bitrate_hz)
{
    (void)bitrate_hz;
    return SSCB_OK;
}

sscb_status_t sscb_i2c_write_reg8(uint8_t target_addr_7bit, uint8_t reg, uint8_t value)
{
    if (target_addr_7bit >= 128u || reg >= 64u) {
        return SSCB_ERR_ARG;
    }

    simulated_registers[target_addr_7bit][reg] = value;
    simulated_write_count++;
    return SSCB_OK;
}

sscb_status_t sscb_i2c_read_reg8(uint8_t target_addr_7bit, uint8_t reg, uint8_t *value)
{
    if (value == 0 || target_addr_7bit >= 128u || reg >= 64u) {
        return SSCB_ERR_ARG;
    }

    *value = simulated_registers[target_addr_7bit][reg];
    return SSCB_OK;
}

uint8_t sscb_i2c_test_peek_reg(uint8_t target_addr_7bit, uint8_t reg)
{
    if (target_addr_7bit >= 128u || reg >= 64u) {
        return 0u;
    }
    return simulated_registers[target_addr_7bit][reg];
}

uint8_t sscb_i2c_test_write_count(void)
{
    return simulated_write_count;
}

void sscb_i2c_test_reset(void)
{
    uint16_t addr;
    uint16_t reg;

    for (addr = 0u; addr < 128u; addr++) {
        for (reg = 0u; reg < 64u; reg++) {
            simulated_registers[addr][reg] = 0u;
        }
    }
    simulated_write_count = 0u;
}
#else
static sscb_status_t wait_until_not_busy(void)
{
    uint32_t guard = SSCB_I2C_TIMEOUT_LOOPS;

    while (I2C_isBusBusy(I2CA_BASE) && guard > 0u) {
        guard--;
    }
    return (guard == 0u) ? SSCB_ERR_TIMEOUT : SSCB_OK;
}

static sscb_status_t wait_for_stop(void)
{
    uint32_t guard = SSCB_I2C_TIMEOUT_LOOPS;

    while (I2C_getStopConditionStatus(I2CA_BASE) && guard > 0u) {
        guard--;
    }
    return (guard == 0u) ? SSCB_ERR_TIMEOUT : SSCB_OK;
}

static sscb_status_t check_i2c_error(void)
{
    uint16_t status = I2C_getStatus(I2CA_BASE);

    if ((status & (I2C_STS_NO_ACK | I2C_STS_ARB_LOST)) != 0u) {
        I2C_clearStatus(I2CA_BASE, I2C_STS_NO_ACK | I2C_STS_ARB_LOST);
        I2C_sendStopCondition(I2CA_BASE);
        return SSCB_ERR_HW;
    }

    return SSCB_OK;
}

sscb_status_t sscb_i2c_driver_init(uint32_t bitrate_hz)
{
    if (bitrate_hz == 0u) {
        return SSCB_ERR_ARG;
    }

    I2C_disableModule(I2CA_BASE);
    I2C_initController(I2CA_BASE, SSCB_CPU_HZ, bitrate_hz, I2C_DUTYCYCLE_50);
    I2C_setBitCount(I2CA_BASE, I2C_BITCOUNT_8);
    I2C_setFIFOInterruptLevel(I2CA_BASE, I2C_FIFO_TXEMPTY, I2C_FIFO_RX1);
    I2C_enableFIFO(I2CA_BASE);
    I2C_enableModule(I2CA_BASE);
    return SSCB_OK;
}

sscb_status_t sscb_i2c_write_reg8(uint8_t target_addr_7bit, uint8_t reg, uint8_t value)
{
    sscb_status_t st;

    if (target_addr_7bit >= 128u) {
        return SSCB_ERR_ARG;
    }

    st = wait_until_not_busy();
    if (st != SSCB_OK) return st;

    I2C_setTargetAddress(I2CA_BASE, target_addr_7bit);
    I2C_setConfig(I2CA_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(I2CA_BASE, 2u);
    I2C_putData(I2CA_BASE, reg);
    I2C_putData(I2CA_BASE, value);
    I2C_sendStartCondition(I2CA_BASE);
    I2C_sendStopCondition(I2CA_BASE);

    st = wait_for_stop();
    if (st != SSCB_OK) return st;

    return check_i2c_error();
}

sscb_status_t sscb_i2c_read_reg8(uint8_t target_addr_7bit, uint8_t reg, uint8_t *value)
{
    sscb_status_t st;
    uint32_t guard;

    if (value == 0 || target_addr_7bit >= 128u) {
        return SSCB_ERR_ARG;
    }

    st = wait_until_not_busy();
    if (st != SSCB_OK) return st;

    I2C_setTargetAddress(I2CA_BASE, target_addr_7bit);
    I2C_setConfig(I2CA_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(I2CA_BASE, 1u);
    I2C_putData(I2CA_BASE, reg);
    I2C_sendStartCondition(I2CA_BASE);

    guard = SSCB_I2C_TIMEOUT_LOOPS;
    while ((I2C_getStatus(I2CA_BASE) & I2C_STS_REG_ACCESS_RDY) == 0u && guard > 0u) {
        guard--;
    }
    if (guard == 0u) return SSCB_ERR_TIMEOUT;
    st = check_i2c_error();
    if (st != SSCB_OK) return st;

    I2C_setConfig(I2CA_BASE, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_setDataCount(I2CA_BASE, 1u);
    I2C_sendStartCondition(I2CA_BASE);
    I2C_sendStopCondition(I2CA_BASE);

    guard = SSCB_I2C_TIMEOUT_LOOPS;
    while ((I2C_getStatus(I2CA_BASE) & I2C_STS_RX_DATA_RDY) == 0u && guard > 0u) {
        guard--;
    }
    if (guard == 0u) return SSCB_ERR_TIMEOUT;

    *value = (uint8_t)I2C_getData(I2CA_BASE);
    return wait_for_stop();
}
#endif
