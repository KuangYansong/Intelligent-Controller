#ifndef EPWM_DRIVER_H
#define EPWM_DRIVER_H

#include <stdbool.h>
#include "sscb_types.h"

/* 初始化 ePWM 输出。 */
SscbStatus EpwmDriver_Init(void);
/* 触发或解除跳闸输出。 */
void EpwmDriver_SetTrip(bool trip);

#endif
