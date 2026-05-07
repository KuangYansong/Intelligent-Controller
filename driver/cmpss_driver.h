#ifndef CMPSS_DRIVER_H
#define CMPSS_DRIVER_H

#include "sscb_types.h"

SscbStatus CmpssDriver_Init(float short_threshold_a);
void CmpssDriver_SetThreshold(float short_threshold_a);

#endif

