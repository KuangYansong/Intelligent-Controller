#ifndef EPWM_DRIVER_H
#define EPWM_DRIVER_H

#include <stdbool.h>
#include "sscb_types.h"

SscbStatus EpwmDriver_Init(void);
void EpwmDriver_SetTrip(bool trip);

#endif

