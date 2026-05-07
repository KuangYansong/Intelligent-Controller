#ifndef SSCB_CONFIG_H
#define SSCB_CONFIG_H

#include <stdint.h>

#define SSCB_FW_VERSION_MAJOR        (1u)
#define SSCB_FW_VERSION_MINOR        (0u)

#define SSCB_ADC_PERIOD_US           (20u)
#define SSCB_CONTROL_PERIOD_MS       (10u)
#define SSCB_REALTIME_PERIOD_MS      (20u)
#define SSCB_HEARTBEAT_PERIOD_MS     (100u)
#define SSCB_RMS_WINDOW_SIZE         (500u)

#define SSCB_CAN_BITRATE             (500000u)
#define SSCB_DEFAULT_NODE_ID         (1u)

#define SSCB_SHORT_RECOVER_DELAY_MS  (5000u)
#define SSCB_OVERLOAD_RECOVER_DELAY_MS (15000u)

#define SSCB_FAULT_LOG_CAPACITY      (50u)

#define SSCB_PARAMS_MAGIC            (0x53534342UL)
#define SSCB_PARAMS_VERSION          (1u)

#define SSCB_FRAM_SIZE_BYTES         (262144UL)
#define SSCB_FRAM_PARAM_ADDR         (0x0000UL)
#define SSCB_FRAM_FAULT_META_ADDR    (0x0200UL)
#define SSCB_FRAM_FAULT_LOG_ADDR     (0x0300UL)

#define SSCB_ADC_MAX_COUNTS          (4095u)
#define SSCB_ADC_REF_MV              (3300u)

#define SSCB_VOLTAGE_SCALE_DEFAULT   (1.0f)
#define SSCB_CURRENT_SCALE_DEFAULT   (1.0f)
#define SSCB_TEMP_SCALE_DEFAULT      (1.0f)

#define SSCB_DEFAULT_SHORT_A         (200.0f)
#define SSCB_DEFAULT_OVERCURRENT_A   (80.0f)
#define SSCB_DEFAULT_OVERVOLTAGE_V   (850.0f)
#define SSCB_DEFAULT_OVERTEMP_C      (85.0f)
#define SSCB_DEFAULT_I2T             (6400.0f)

#endif

