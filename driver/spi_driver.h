#ifndef SSCB_SPI_DRIVER_H
#define SSCB_SPI_DRIVER_H

#include <stdint.h>
#include "common/sscb_types.h"

sscb_status_t sscb_spi_driver_init(void);
sscb_status_t sscb_spi_transfer(const uint8_t *tx, uint8_t *rx, uint16_t len);

#endif
