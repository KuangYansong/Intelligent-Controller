#ifndef CMPSS_DRIVER_H
#define CMPSS_DRIVER_H

#include "sscb_types.h"

/* 初始化短路比较器并设置初始门限。 */
SscbStatus CmpssDriver_Init(float short_threshold_a);
/* 动态调整短路比较器门限。 */
void CmpssDriver_SetThreshold(float short_threshold_a);

#endif
