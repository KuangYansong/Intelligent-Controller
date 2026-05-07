#ifndef PARAM_STORE_H
#define PARAM_STORE_H

#include "sscb_types.h"

/* 恢复默认参数。 */
void ParamStore_Defaults(SscbParams *params);
/* 从 FRAM 载入参数。 */
SscbStatus ParamStore_Load(SscbParams *params);
/* 把当前参数写回 FRAM。 */
SscbStatus ParamStore_Save(SscbParams *params);
/* 按参数编号读取一个参数值。 */
SscbStatus ParamStore_Get(const SscbParams *params, SscbParamId id, float *value);
/* 按参数编号修改参数值并立即保存。 */
SscbStatus ParamStore_Set(SscbParams *params, SscbParamId id, float value);

#endif
