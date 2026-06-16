#ifndef SSCB_GATE_DRIVER_H
#define SSCB_GATE_DRIVER_H

#include "common/sscb_types.h"

enum {
    SSCB_GD_REG_I2CADD = 0x00u,
    SSCB_GD_REG_I2CGADD = 0x01u,
    SSCB_GD_REG_I2CCFGOK = 0x02u,
    SSCB_GD_REG_UVTLVL = 0x06u,
    SSCB_GD_REG_D1LVL = 0x0Cu,
    SSCB_GD_REG_D1FILT = 0x0Du,
    SSCB_GD_REG_DLEBT = 0x12u,
    SSCB_GD_REG_DRVFOFF = 0x15u,
    SSCB_GD_REG_DRVCFG = 0x16u,
    SSCB_GD_REG_TLTOC1 = 0x17u,
    SSCB_GD_REG_TLTOC2 = 0x18u,
    SSCB_GD_REG_CSSOFCFG = 0x19u,
    SSCB_GD_REG_CLCFG = 0x1Au,
    SSCB_GD_REG_CFGOK = 0x1Cu
};

typedef struct {
    uint8_t i2c_addr_7bit;
    uint8_t group_addr_7bit;
    uint8_t uvtlvl;
    uint8_t d1lvl;
    uint8_t d1filt;
    uint8_t dlebt;
    uint8_t drvfoff;
    uint8_t drvcfg;
    uint8_t tltoc1;
    uint8_t tltoc2;
    uint8_t cssofcfg;
    uint8_t clcfg;
} sscb_gate_driver_config_t;

sscb_gate_driver_config_t sscb_gate_driver_default_config(void);
sscb_status_t sscb_gate_driver_init(void);
sscb_status_t sscb_gate_driver_apply_config(const sscb_gate_driver_config_t *config);
sscb_status_t sscb_gate_driver_verify_config(const sscb_gate_driver_config_t *config);
uint8_t sscb_gate_driver_expected_register(uint8_t reg, const sscb_gate_driver_config_t *config);

#endif
