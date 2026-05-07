#include "fault_log.h"
#include "fram.h"
#include "sscb_config.h"

typedef struct
{
    uint16_t magic;
    uint8_t head;
    uint8_t count;
} FaultLogMeta;

#define FAULT_LOG_META_MAGIC (0x464Cu)

static FaultLogMeta s_meta;

static uint32_t record_addr(uint8_t slot)
{
    return SSCB_FRAM_FAULT_LOG_ADDR + ((uint32_t)slot * (uint32_t)sizeof(SscbFaultRecord));
}

static SscbStatus save_meta(void)
{
    return Fram_Write(SSCB_FRAM_FAULT_META_ADDR, &s_meta, sizeof(s_meta));
}

SscbStatus FaultLog_Init(void)
{
    SscbStatus status = Fram_Read(SSCB_FRAM_FAULT_META_ADDR, &s_meta, sizeof(s_meta));
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

    SscbStatus status = Fram_Write(record_addr(s_meta.head), record, sizeof(*record));
    if (status != SSCB_OK)
    {
        return status;
    }

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

    uint8_t oldest = (s_meta.count == SSCB_FAULT_LOG_CAPACITY) ? s_meta.head : 0u;
    uint8_t slot = (uint8_t)((oldest + index) % SSCB_FAULT_LOG_CAPACITY);
    return Fram_Read(record_addr(slot), record, sizeof(*record));
}

uint8_t FaultLog_Count(void)
{
    return s_meta.count;
}

