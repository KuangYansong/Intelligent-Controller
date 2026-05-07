#ifndef SSCB_TYPES_H
#define SSCB_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    /* 成功。 */
    SSCB_OK = 0,
    /* 通用错误。 */
    SSCB_ERROR = -1,
    /* 参数为空或超范围。 */
    SSCB_BAD_PARAM = -2,
    /* CRC 校验失败。 */
    SSCB_CRC_ERROR = -3,
    /* 底层资源尚未准备好。 */
    SSCB_NOT_READY = -4,
    /* 查找的对象不存在。 */
    SSCB_NOT_FOUND = -5
} SscbStatus;

typedef enum
{
    /* 正常运行。 */
    SSCB_STATE_NORMAL = 0,
    /* 故障已触发，当前处于跳闸状态。 */
    SSCB_STATE_FAULT_ACTIVE = 1,
    /* 故障后等待恢复条件满足。 */
    SSCB_STATE_RECOVER_WAIT = 2,
    /* 正在尝试恢复输出。 */
    SSCB_STATE_RECOVER_TRY = 3,
    /* 锁定态，需要人工干预。 */
    SSCB_STATE_LOCKOUT = 4
} SscbSystemState;

typedef enum
{
    /* 无故障。 */
    SSCB_FAULT_NONE = 0x00,
    /* 短路故障。 */
    SSCB_FAULT_SHORT = 0x01,
    /* 过载故障。 */
    SSCB_FAULT_OVERLOAD = 0x02,
    /* 过压故障。 */
    SSCB_FAULT_OVERVOLTAGE = 0x03,
    /* 过温故障。 */
    SSCB_FAULT_OVERTEMP = 0x04,
    /* 采样异常。 */
    SSCB_FAULT_SAMPLE = 0x05,
    /* 通信超时。 */
    SSCB_FAULT_COMM_TIMEOUT = 0x06,
    /* 自检失败。 */
    SSCB_FAULT_SELFTEST = 0x07
} SscbFaultCode;

typedef enum
{
    SSCB_PARAM_SHORT_THRESHOLD = 0x01,
    SSCB_PARAM_OVERCURRENT_THRESHOLD = 0x02,
    SSCB_PARAM_OVERVOLTAGE_THRESHOLD = 0x03,
    SSCB_PARAM_OVERTEMP_THRESHOLD = 0x04,
    SSCB_PARAM_I2T_THRESHOLD = 0x05,
    SSCB_PARAM_VOLTAGE_K = 0x06,
    SSCB_PARAM_CURRENT_K = 0x07,
    SSCB_PARAM_TEMP_K = 0x08,
    SSCB_PARAM_NODE_ID = 0x09,
    SSCB_PARAM_RECOVER_MODE = 0x0A
} SscbParamId;

typedef struct
{
    /* ADC 原始电压采样计数。 */
    uint16_t voltage_raw;
    /* ADC 原始电流采样计数。 */
    uint16_t current_raw;
    /* ADC 原始温度采样计数。 */
    uint16_t temp_raw;
} SscbAdcRaw;

typedef struct
{
    /* 换算后的电压值。 */
    float voltage_v;
    /* 瞬时电流值。 */
    float current_a;
    /* 电流有效值。 */
    float current_rms_a;
    /* 温度值。 */
    float temperature_c;
} SscbMeasurements;

typedef struct
{
    /* 故障类型。 */
    SscbFaultCode fault;
    /* 故障发生时间戳。 */
    uint32_t timestamp_ms;
    /* 故障发生时的电压。 */
    float voltage_v;
    /* 故障发生时的电流。 */
    float current_a;
    /* 故障发生时的温度。 */
    float temperature_c;
} SscbFaultRecord;

typedef struct
{
    /* 用于识别参数块是否合法。 */
    uint32_t magic;
    /* 参数结构版本号。 */
    uint16_t version;
    /* 本节点的 CAN 节点号。 */
    uint8_t node_id;
    /* 恢复模式配置。 */
    uint8_t recover_mode;
    /* 短路阈值。 */
    float short_threshold_a;
    /* 过流阈值。 */
    float overcurrent_threshold_a;
    /* 过压阈值。 */
    float overvoltage_threshold_v;
    /* 过温阈值。 */
    float overtemp_threshold_c;
    /* I2T 过载阈值。 */
    float i2t_threshold;
    /* 电压校准系数。 */
    float voltage_k;
    /* 电流校准系数。 */
    float current_k;
    /* 温度校准系数。 */
    float temp_k;
    /* 参数块 CRC 校验值。 */
    uint16_t crc16;
} SscbParams;

#endif
