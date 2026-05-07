#ifndef FRAM_H
#define FRAM_H

#include <stddef.h>
#include <stdint.h>
#include "sscb_types.h"

/* 初始化 FRAM 抽象层。 */
SscbStatus Fram_Init(void);
/* 从 FRAM 读取指定地址范围。 */
SscbStatus Fram_Read(uint32_t address, void *data, size_t len);
/* 向 FRAM 写入指定地址范围。 */
SscbStatus Fram_Write(uint32_t address, const void *data, size_t len);

#endif
