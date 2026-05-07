#include "param_store.h"
#include <stddef.h>
#include "crc16.h"
#include "fram.h"
#include "sscb_config.h"

static uint16_t params_crc(const SscbParams *params)
{
    return Crc16_CcittFalse(params, offsetof(SscbParams, crc16));
}

void ParamStore_Defaults(SscbParams *params)
{
    if (params == 0)
    {
        return;
    }

    params->magic = SSCB_PARAMS_MAGIC;
    params->version = SSCB_PARAMS_VERSION;
    params->node_id = SSCB_DEFAULT_NODE_ID;
    params->recover_mode = 0u;
    params->short_threshold_a = SSCB_DEFAULT_SHORT_A;
    params->overcurrent_threshold_a = SSCB_DEFAULT_OVERCURRENT_A;
    params->overvoltage_threshold_v = SSCB_DEFAULT_OVERVOLTAGE_V;
    params->overtemp_threshold_c = SSCB_DEFAULT_OVERTEMP_C;
    params->i2t_threshold = SSCB_DEFAULT_I2T;
    params->voltage_k = SSCB_VOLTAGE_SCALE_DEFAULT;
    params->current_k = SSCB_CURRENT_SCALE_DEFAULT;
    params->temp_k = SSCB_TEMP_SCALE_DEFAULT;
    params->crc16 = params_crc(params);
}

SscbStatus ParamStore_Load(SscbParams *params)
{
    if (params == 0)
    {
        return SSCB_BAD_PARAM;
    }

    SscbStatus status = Fram_Read(SSCB_FRAM_PARAM_ADDR, params, sizeof(*params));
    if (status != SSCB_OK)
    {
        return status;
    }

    if ((params->magic != SSCB_PARAMS_MAGIC) ||
        (params->version != SSCB_PARAMS_VERSION) ||
        (params->crc16 != params_crc(params)) ||
        (params->node_id == 0u) ||
        (params->node_id > 127u))
    {
        ParamStore_Defaults(params);
        return ParamStore_Save(params);
    }

    return SSCB_OK;
}

SscbStatus ParamStore_Save(SscbParams *params)
{
    if (params == 0)
    {
        return SSCB_BAD_PARAM;
    }

    params->magic = SSCB_PARAMS_MAGIC;
    params->version = SSCB_PARAMS_VERSION;
    params->crc16 = params_crc(params);
    return Fram_Write(SSCB_FRAM_PARAM_ADDR, params, sizeof(*params));
}

SscbStatus ParamStore_Get(const SscbParams *params, SscbParamId id, float *value)
{
    if ((params == 0) || (value == 0))
    {
        return SSCB_BAD_PARAM;
    }

    switch (id)
    {
    case SSCB_PARAM_SHORT_THRESHOLD:
        *value = params->short_threshold_a;
        break;
    case SSCB_PARAM_OVERCURRENT_THRESHOLD:
        *value = params->overcurrent_threshold_a;
        break;
    case SSCB_PARAM_OVERVOLTAGE_THRESHOLD:
        *value = params->overvoltage_threshold_v;
        break;
    case SSCB_PARAM_OVERTEMP_THRESHOLD:
        *value = params->overtemp_threshold_c;
        break;
    case SSCB_PARAM_I2T_THRESHOLD:
        *value = params->i2t_threshold;
        break;
    case SSCB_PARAM_VOLTAGE_K:
        *value = params->voltage_k;
        break;
    case SSCB_PARAM_CURRENT_K:
        *value = params->current_k;
        break;
    case SSCB_PARAM_TEMP_K:
        *value = params->temp_k;
        break;
    case SSCB_PARAM_NODE_ID:
        *value = (float)params->node_id;
        break;
    case SSCB_PARAM_RECOVER_MODE:
        *value = (float)params->recover_mode;
        break;
    default:
        return SSCB_NOT_FOUND;
    }

    return SSCB_OK;
}

SscbStatus ParamStore_Set(SscbParams *params, SscbParamId id, float value)
{
    if (params == 0)
    {
        return SSCB_BAD_PARAM;
    }

    if (value < 0.0f)
    {
        return SSCB_BAD_PARAM;
    }

    switch (id)
    {
    case SSCB_PARAM_SHORT_THRESHOLD:
        params->short_threshold_a = value;
        break;
    case SSCB_PARAM_OVERCURRENT_THRESHOLD:
        params->overcurrent_threshold_a = value;
        break;
    case SSCB_PARAM_OVERVOLTAGE_THRESHOLD:
        params->overvoltage_threshold_v = value;
        break;
    case SSCB_PARAM_OVERTEMP_THRESHOLD:
        params->overtemp_threshold_c = value;
        break;
    case SSCB_PARAM_I2T_THRESHOLD:
        params->i2t_threshold = value;
        break;
    case SSCB_PARAM_VOLTAGE_K:
        params->voltage_k = value;
        break;
    case SSCB_PARAM_CURRENT_K:
        params->current_k = value;
        break;
    case SSCB_PARAM_TEMP_K:
        params->temp_k = value;
        break;
    case SSCB_PARAM_NODE_ID:
        if ((value < 1.0f) || (value > 127.0f))
        {
            return SSCB_BAD_PARAM;
        }
        params->node_id = (uint8_t)value;
        break;
    case SSCB_PARAM_RECOVER_MODE:
        if (value > 255.0f)
        {
            return SSCB_BAD_PARAM;
        }
        params->recover_mode = (uint8_t)value;
        break;
    default:
        return SSCB_NOT_FOUND;
    }

    return ParamStore_Save(params);
}

