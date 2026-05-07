#ifndef PARAM_STORE_H
#define PARAM_STORE_H

#include "sscb_types.h"

void ParamStore_Defaults(SscbParams *params);
SscbStatus ParamStore_Load(SscbParams *params);
SscbStatus ParamStore_Save(SscbParams *params);
SscbStatus ParamStore_Get(const SscbParams *params, SscbParamId id, float *value);
SscbStatus ParamStore_Set(SscbParams *params, SscbParamId id, float value);

#endif

