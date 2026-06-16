#ifndef SSCB_EPWM_DRIVER_H
#define SSCB_EPWM_DRIVER_H

#include <stdbool.h>
#include "common/sscb_types.h"

sscb_status_t sscb_epwm_driver_init_trip_chain(void);
void sscb_epwm_force_off(void);
void sscb_epwm_allow_output(bool enable);
bool sscb_epwm_output_enabled(void);

#endif
