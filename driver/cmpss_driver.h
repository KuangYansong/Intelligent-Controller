#ifndef SSCB_CMPSS_DRIVER_H
#define SSCB_CMPSS_DRIVER_H

#include <stdint.h>
#include "common/sscb_types.h"

sscb_status_t sscb_cmpss_driver_init(uint16_t dac_code);
sscb_status_t sscb_cmpss_driver_set_dac(uint16_t dac_code);
uint16_t sscb_cmpss_driver_last_dac(void);

#endif
