#include "rms.h"
#include <math.h>

void Rms_Init(RmsWindow *rms)
{
    if (rms == 0)
    {
        return;
    }

    /* 采样窗口里的平方值全部清零，相当于重新开始统计。 */
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

    /* RMS = sqrt(平方平均值)，所以先把当前采样转成平方值。 */
    float square = current_a * current_a;
    /* 用环形缓冲区维护固定长度窗口，移除最旧值，再加入最新值。 */
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

    /* 有效值就是平方平均后再开方。 */
    return sqrtf(rms->sum_squares / (float)rms->count);
}
