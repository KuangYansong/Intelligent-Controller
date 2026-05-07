#ifndef FAULT_LOG_H
#define FAULT_LOG_H

#include <stdint.h>
#include "sscb_types.h"

SscbStatus FaultLog_Init(void);
SscbStatus FaultLog_Append(const SscbFaultRecord *record);
SscbStatus FaultLog_Read(uint8_t index, SscbFaultRecord *record);
uint8_t FaultLog_Count(void);

#endif

