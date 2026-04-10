#ifndef IMU_TASK_H
#define IMU_TASK_H

#include <freertos/FreeRTOS.h>
#include <tasks.h>
#include <cstdint>

#include <queue.h>
#include "shared.h"            // brings imuQueue declaration

/* =========================================================
   BNO055 Constants
   ========================================================= */



#define BNO055_REG_CHIP_ID        0x00
#define BNO055_REG_OPR_MODE       0x3D
#define BNO055_REG_PWR_MODE       0x3E
#define BNO055_REG_UNIT_SEL       0x3B
#define BNO055_REG_SYS_TRIGGER    0x3F
#define BNO055_REG_CALIB_STAT     0x35
#define BNO055_REG_QUAT_LSB       0x20

#define BNO055_CHIP_ID_VALUE      0xA0


#define BNO055_TX_PIN 43   // MCU TX -> BNO RX
#define BNO055_RX_PIN 44   // MCU RX <- BNO TX
#define BNO055_RESET_PIN 7 // Active Low
#define BNO055_BAUD 115200
#define BNO055_START_BYTE         0xAA
#define BNO055_WRITE_CMD          0x00
#define BNO055_READ_CMD           0x01
#define BNO055_RESP_WRITE         0xEE
#define BNO055_RESP_READ          0xBB
#define BNO055_WRITE_SUCCESS      0x01

#define BNO055_MODE_CONFIG        0x00
#define BNO055_MODE_NDOF          0x0C

#define BNO055_TRIGGER_RESET      0x20
#define BNO055_TRIGGER_EXT_CRYST  0x80

#define BNO055_UNIT_SEL_DEFAULT   ((0x00 << 0) |  /* m/s² */ \
                                    (0x01 << 1) |  /* rad/s */ \
                                    (0x01 << 2) |  /* radians */ \
                                    (0x00 << 4) |  /* °C */ \
                                    (0x00 << 7))   /* Windows */

#define BNO055_DELAY_RESET_MS        650
#define BNO055_DELAY_MODE_SWITCH_MS  7
#define BNO055_DELAY_CONFIG_MS       20
#define BNO055_UART_TIMEOUT_MS       30

#define BNO055_QUAT_SCALE (1.0f / 16384.0f)

/* =========================================================
   Sampling Configuration
   ========================================================= */


#define IMU_SAMPLE_RATE_HZ  100
#define IMU_TASK_PERIOD_MS  (1000 / IMU_SAMPLE_RATE_HZ)

/* =========================================================
   Data Structures & Task Declaration
   ========================================================= */

typedef struct
{
    float w;
    float x;
    float y;
    float z;
    uint32_t timestamp_ms;
} IMUSample_t;

typedef struct
{
    float w;
    float x;
    float y;
    float z;
    uint32_t timestamp_ms;
} imu_attitude_t;

#endif /* IMU_TASK_H */