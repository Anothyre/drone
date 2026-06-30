#ifndef IMU_TASK_H
#define IMU_TASK_H

#include <freertos/FreeRTOS.h>
#include <cstdint>

#define BNO055_TX_PIN 43
#define BNO055_RX_PIN 44
#define BNO055_RESET_PIN 7
#define BNO055_BAUD 115200

#define BNO055_UART_START_BYTE 0xAA
#define BNO055_UART_WRITE_CMD 0x00
#define BNO055_UART_READ_CMD 0x01
#define BNO055_UART_RESP_WRITE 0xEE
#define BNO055_UART_RESP_READ 0xBB
#define BNO055_UART_WRITE_SUCCESS 0x01

#define BNO055_UART_TIMEOUT_MS 100
#define BNO055_BOOT_DELAY_MS 800
#define BNO055_RESET_LOW_MS 10
#define BNO055_QUAT_SCALE (1.0f / 16384.0f)

#define IMU_SAMPLE_RATE_HZ 100
#define IMU_TASK_PERIOD_MS (1000 / IMU_SAMPLE_RATE_HZ)

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
