#include "rms.h"
#include <math.h>

void Rms_Init(RmsWindow *rms)
{
    if (rms == 0)
    {
        return;
    }

    for (uint16_t i = 0u; i < SSCB_RMS_WINDOW_SIZE; i++)
    {
        rms->squares[i] = 0.0f;
    }
    rms->sum_squares = 0.0f;
    rms->index = 0u;
    rms->count = 0u;
}

float Rms_AddSample(RmsWindow *rms, float current_a)
{
    if (rms == 0)
    {
        return 0.0f;
    }

    float square = current_a * current_a;
    rms->sum_squares -= rms->squares[rms->index];
    rms->squares[rms->index] = square;
    rms->sum_squares += square;
    rms->index = (uint16_t)((rms->index + 1u) % SSCB_RMS_WINDOW_SIZE);

    if (rms->count < SSCB_RMS_WINDOW_SIZE)
    {
        rms->count++;
    }

    return Rms_Get(rms);
}

float Rms_Get(const RmsWindow *rms)
{
    if ((rms == 0) || (rms->count == 0u))
    {
        return 0.0f;
    }

    return sqrtf(rms->sum_squares / (float)rms->count);
}

