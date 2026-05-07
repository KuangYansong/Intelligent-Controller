#include "fault_log.h"
#include "fram.h"
#include "sscb_config.h"

typedef struct
{
    /* 用于判断元数据是否有效。 */
    uint16_t magic;
    /* 下一个写入位置。 */
    uint8_t head;
    /* 当前已经保存了多少条记录。 */
    uint8_t count;
} FaultLogMeta;

#define FAULT_LOG_META_MAGIC (0x464Cu)

static FaultLogMeta s_meta;

static uint32_t record_addr(uint8_t slot)
{
    /* 每个槽位大小固定，所以地址可按“起始地址 + 槽位号 * 记录大小”计算。 */
    return SSCB_FRAM_FAULT_LOG_ADDR + ((uint32_t)slot * (uint32_t)sizeof(SscbFaultRecord));
}

static SscbStatus save_meta(void)
{
    /* 元数据和真实记录分开保存，便于快速定位头指针和数量。 */
    return Fram_Write(SSCB_FRAM_FAULT_META_ADDR, &s_meta, sizeof(s_meta));
}

SscbStatus FaultLog_Init(void)
{
    SscbStatus status = Fram_Read(SSCB_FRAM_FAULT_META_ADDR, &s_meta, sizeof(s_meta));
    /* 元数据损坏时，直接重建一个空日志。 */
    if ((status != SSCB_OK) ||
        (s_meta.magic != FAULT_LOG_META_MAGIC) ||
        (s_meta.head >= SSCB_FAULT_LOG_CAPACITY) ||
        (s_meta.count > SSCB_FAULT_LOG_CAPACITY))
    {
        s_meta.magic = FAULT_LOG_META_MAGIC;
        s_meta.head = 0u;
        s_meta.count = 0u;
        status = save_meta();
    }
    return status;
}

SscbStatus FaultLog_Append(const SscbFaultRecord *record)
{
    if (record == 0)
    {
        return SSCB_BAD_PARAM;
    }

    /* 新记录永远写到 head 指向的位置，这本质上是环形队列。 */
    SscbStatus status = Fram_Write(record_addr(s_meta.head), record, sizeof(*record));
    if (status != SSCB_OK)
    {
        return status;
    }

    /* 写完后 head 前移；满了以后就覆盖最旧记录。 */
    s_meta.head = (uint8_t)((s_meta.head + 1u) % SSCB_FAULT_LOG_CAPACITY);
    if (s_meta.count < SSCB_FAULT_LOG_CAPACITY)
    {
        s_meta.count++;
    }
    return save_meta();
}

SscbStatus FaultLog_Read(uint8_t index, SscbFaultRecord *record)
{
    if ((record == 0) || (index >= s_meta.count))
    {
        return SSCB_BAD_PARAM;
    }

    /* 日志满时，最旧记录从 head 开始；没满时，最旧记录从 0 开始。 */
    uint8_t oldest = (s_meta.count == SSCB_FAULT_LOG_CAPACITY) ? s_meta.head : 0u;
    uint8_t slot = (uint8_t)((oldest + index) % SSCB_FAULT_LOG_CAPACITY);
    return Fram_Read(record_addr(slot), record, sizeof(*record));
}

SscbStatus FaultLog_Clear(void)
{
    /* 清空时不必擦除全部记录，只需把元数据重置为空队列。 */
    s_meta.magic = FAULT_LOG_META_MAGIC;
    s_meta.head = 0u;
    s_meta.count = 0u;
    return save_meta();
}

uint8_t FaultLog_Count(void)
{
    /* 返回当前日志中有效记录数量。 */
    return s_meta.count;
}
