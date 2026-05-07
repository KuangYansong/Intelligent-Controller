#ifndef RMS_H
#define RMS_H

#include <stdint.h>
#include "sscb_config.h"

typedef struct
{
    float squares[SSCB_RMS_WINDOW_SIZE];
    float sum_squares;
    uint16_t index;
    uint16_t count;
} RmsWindow;

void Rms_Init(RmsWindow *rms);
float Rms_AddSample(RmsWindow *rms, float current_a);
float Rms_Get(const RmsWindow *rms);

#endif

