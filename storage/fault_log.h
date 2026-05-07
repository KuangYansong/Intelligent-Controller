#ifndef FAULT_LOG_H
#define FAULT_LOG_H

#include <stdint.h>
#include "sscb_types.h"

/* 初始化故障日志元数据。 */
SscbStatus FaultLog_Init(void);
/* 追加一条新的故障记录。 */
SscbStatus FaultLog_Append(const SscbFaultRecord *record);
/* 按时间顺序读取第 index 条故障。 */
SscbStatus FaultLog_Read(uint8_t index, SscbFaultRecord *record);
/* 清空故障日志。 */
SscbStatus FaultLog_Clear(void);
/* 返回当前已记录的故障条数。 */
uint8_t FaultLog_Count(void);

#endif
