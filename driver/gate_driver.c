#include "driver/gate_driver.h"
#include "bsp/board_resources.h"
#include "driver/i2c_driver.h"

typedef struct {
    uint8_t reg;
    uint8_t value;
} sscb_gate_driver_reg_write_t;

static uint8_t tltoc1_value(uint8_t plateau_v_x4, uint8_t ramp_a_code)
{
    uint8_t level_code = (uint8_t)(plateau_v_x4 - 17u);
    return (uint8_t)((level_code << 2) | (ramp_a_code & 0x03u));
}

static uint8_t tltoc2_value(uint8_t ramp_b_code, uint8_t time_250ns_count)
{
    return (uint8_t)(((ramp_b_code & 0x07u) << 5) | (time_250ns_count & 0x1Fu));
}

sscb_gate_driver_config_t sscb_gate_driver_default_config(void)
{
    sscb_gate_driver_config_t config;

    config.i2c_addr_7bit = SSCB_GATE_DRIVER_I2C_ADDR_7BIT;
    config.group_addr_7bit = 0x0Eu;
    config.uvtlvl = 0x05u;                  /* VCC2 MOSFET, VEE2 high setting for -5 V supply. */
    config.d1lvl = 0x1Fu;                   /* DESAT enabled, DESAT1 threshold 9.18 V. */
    config.d1filt = 0x10u;                  /* 475 ns, nearest register step to 500 ns, up/reset counter. */
    config.dlebt = 0x12u;                   /* 100 ns + 18 * 50 ns = 1.0 us. */
    config.drvfoff = 0x02u;                 /* Fault switch-off = TLTOff. */
    config.drvcfg = 0x00u;                  /* Normal switch-off = hard, high gate charge range. */
    config.tltoc1 = tltoc1_value(36u, 2u);  /* 9.0 V plateau, ramp A 30 V/us. */
    config.tltoc2 = tltoc2_value(6u, 6u);   /* Ramp B 30 V/us, plateau time 1.5 us. */
    config.cssofcfg = 0x09u;                /* Soft-off current default, about 7.3%. */
    config.clcfg = 0x22u;                   /* 235 ns up/reset, external CLAMP pre-driver, enabled. */
    return config;
}

sscb_status_t sscb_gate_driver_init(void)
{
    return sscb_i2c_driver_init(SSCB_I2CA_BITRATE);
}

uint8_t sscb_gate_driver_expected_register(uint8_t reg, const sscb_gate_driver_config_t *config)
{
    sscb_gate_driver_config_t defaults;
    const sscb_gate_driver_config_t *cfg = config;

    if (cfg == 0) {
        defaults = sscb_gate_driver_default_config();
        cfg = &defaults;
    }

    switch (reg) {
    case SSCB_GD_REG_I2CADD: return cfg->i2c_addr_7bit;
    case SSCB_GD_REG_I2CGADD: return cfg->group_addr_7bit;
    case SSCB_GD_REG_I2CCFGOK: return 0x01u;
    case SSCB_GD_REG_UVTLVL: return cfg->uvtlvl;
    case SSCB_GD_REG_D1LVL: return cfg->d1lvl;
    case SSCB_GD_REG_D1FILT: return cfg->d1filt;
    case SSCB_GD_REG_DLEBT: return cfg->dlebt;
    case SSCB_GD_REG_DRVFOFF: return cfg->drvfoff;
    case SSCB_GD_REG_DRVCFG: return cfg->drvcfg;
    case SSCB_GD_REG_TLTOC1: return cfg->tltoc1;
    case SSCB_GD_REG_TLTOC2: return cfg->tltoc2;
    case SSCB_GD_REG_CSSOFCFG: return cfg->cssofcfg;
    case SSCB_GD_REG_CLCFG: return cfg->clcfg;
    case SSCB_GD_REG_CFGOK: return 0x01u;
    default: return 0u;
    }
}

sscb_status_t sscb_gate_driver_apply_config(const sscb_gate_driver_config_t *config)
{
    sscb_gate_driver_config_t defaults;
    const sscb_gate_driver_config_t *cfg = config;
    sscb_status_t st;
    sscb_gate_driver_reg_write_t writes[13u];
    uint16_t i;

    if (cfg == 0) {
        defaults = sscb_gate_driver_default_config();
        cfg = &defaults;
    }
    if (cfg->i2c_addr_7bit >= 128u || cfg->group_addr_7bit >= 128u ||
        cfg->i2c_addr_7bit == cfg->group_addr_7bit) {
        return SSCB_ERR_ARG;
    }

    st = sscb_i2c_write_reg8(SSCB_GATE_DRIVER_I2C_ADDR_7BIT, SSCB_GD_REG_I2CADD, cfg->i2c_addr_7bit);
    if (st != SSCB_OK) return st;
    st = sscb_i2c_write_reg8(SSCB_GATE_DRIVER_I2C_ADDR_7BIT, SSCB_GD_REG_I2CGADD, cfg->group_addr_7bit);
    if (st != SSCB_OK) return st;
    st = sscb_i2c_write_reg8(SSCB_GATE_DRIVER_I2C_ADDR_7BIT, SSCB_GD_REG_I2CCFGOK, 0x01u);
    if (st != SSCB_OK) return st;

    writes[0].reg = SSCB_GD_REG_UVTLVL; writes[0].value = cfg->uvtlvl;
    writes[1].reg = SSCB_GD_REG_D1LVL; writes[1].value = cfg->d1lvl;
    writes[2].reg = SSCB_GD_REG_D1FILT; writes[2].value = cfg->d1filt;
    writes[3].reg = SSCB_GD_REG_DLEBT; writes[3].value = cfg->dlebt;
    writes[4].reg = SSCB_GD_REG_DRVFOFF; writes[4].value = cfg->drvfoff;
    writes[5].reg = SSCB_GD_REG_DRVCFG; writes[5].value = cfg->drvcfg;
    writes[6].reg = SSCB_GD_REG_TLTOC1; writes[6].value = cfg->tltoc1;
    writes[7].reg = SSCB_GD_REG_TLTOC2; writes[7].value = cfg->tltoc2;
    writes[8].reg = SSCB_GD_REG_CSSOFCFG; writes[8].value = cfg->cssofcfg;
    writes[9].reg = SSCB_GD_REG_CLCFG; writes[9].value = cfg->clcfg;
    writes[10].reg = SSCB_GD_REG_CFGOK; writes[10].value = 0x01u;
    writes[11].reg = 0u; writes[11].value = 0u;
    writes[12].reg = 0u; writes[12].value = 0u;

    for (i = 0u; i < 11u; i++) {
        st = sscb_i2c_write_reg8(cfg->i2c_addr_7bit, writes[i].reg, writes[i].value);
        if (st != SSCB_OK) {
            return st;
        }
    }

    return SSCB_OK;
}

sscb_status_t sscb_gate_driver_verify_config(const sscb_gate_driver_config_t *config)
{
    sscb_gate_driver_config_t defaults;
    const sscb_gate_driver_config_t *cfg = config;
    uint8_t regs[] = {
        SSCB_GD_REG_UVTLVL, SSCB_GD_REG_D1LVL, SSCB_GD_REG_D1FILT,
        SSCB_GD_REG_DLEBT, SSCB_GD_REG_DRVFOFF, SSCB_GD_REG_DRVCFG,
        SSCB_GD_REG_TLTOC1, SSCB_GD_REG_TLTOC2, SSCB_GD_REG_CSSOFCFG,
        SSCB_GD_REG_CLCFG, SSCB_GD_REG_CFGOK
    };
    uint16_t i;
    uint8_t actual = 0u;

    if (cfg == 0) {
        defaults = sscb_gate_driver_default_config();
        cfg = &defaults;
    }

    for (i = 0u; i < (uint16_t)(sizeof(regs) / sizeof(regs[0])); i++) {
        if (sscb_i2c_read_reg8(cfg->i2c_addr_7bit, regs[i], &actual) != SSCB_OK) {
            return SSCB_ERR_HW;
        }
        if (actual != sscb_gate_driver_expected_register(regs[i], cfg)) {
            return SSCB_ERR_HW;
        }
    }

    return SSCB_OK;
}
