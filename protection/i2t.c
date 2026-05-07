#include "i2t.h"

void I2t_Init(I2tState *state)
{
    if (state != 0)
    {
        state->accumulator = 0.0f;
    }
}

bool I2t_Update(I2tState *state, float current_rms_a, float overcurrent_a, float threshold, float dt_s)
{
    if ((state == 0) || (threshold <= 0.0f) || (overcurrent_a <= 0.0f))
    {
        return false;
    }

    if (current_rms_a > overcurrent_a)
    {
        float excess = current_rms_a - overcurrent_a;
        state->accumulator += excess * excess * dt_s;
    }
    else
    {
        state->accumulator -= overcurrent_a * overcurrent_a * dt_s * 0.05f;
        if (state->accumulator < 0.0f)
        {
            state->accumulator = 0.0f;
        }
    }

    return state->accumulator >= threshold;
}

void I2t_Reset(I2tState *state)
{
    if (state != 0)
    {
        state->accumulator = 0.0f;
    }
}

