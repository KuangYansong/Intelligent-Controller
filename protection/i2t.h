#ifndef I2T_H
#define I2T_H

#include <stdbool.h>

typedef struct
{
    float accumulator;
} I2tState;

void I2t_Init(I2tState *state);
bool I2t_Update(I2tState *state, float current_rms_a, float overcurrent_a, float threshold, float dt_s);
void I2t_Reset(I2tState *state);

#endif

