#ifndef SSCB_I2C_DRIVER_H
#define SSCB_I2C_DRIVER_H

#include "common/sscb_types.h"

sscb_status_t sscb_i2c_driver_init(uint32_t bitrate_hz);
sscb_status_t sscb_i2c_write_reg8(uint8_t target_addr_7bit, uint8_t reg, uint8_t value);
sscb_status_t sscb_i2c_read_reg8(uint8_t target_addr_7bit, uint8_t reg, uint8_t *value);

#ifndef __TMS320C28XX__
uint8_t sscb_i2c_test_peek_reg(uint8_t target_addr_7bit, uint8_t reg);
uint8_t sscb_i2c_test_write_count(void);
void sscb_i2c_test_reset(void);
#endif

#endif
