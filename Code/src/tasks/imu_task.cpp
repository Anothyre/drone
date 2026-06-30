#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks/imu_task.h"
#include "shared.h"
#include <cstring>

extern "C" {
#include "bno055.h"
}

static struct bno055_t g_bno055;

static void flushBnoRx()
{
    while (Serial2.available() > 0) {
        Serial2.read();
    }
}

static bool readByteTimeout(uint8_t *out, uint32_t timeoutMs = BNO055_UART_TIMEOUT_MS)
{
    const TickType_t start = xTaskGetTickCount();
    const TickType_t timeoutTicks = pdMS_TO_TICKS(timeoutMs);

    while ((xTaskGetTickCount() - start) <= timeoutTicks) {
        if (Serial2.available() > 0) {
            *out = static_cast<uint8_t>(Serial2.read());
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return false;
}

static int8_t bno_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    (void)dev_addr;

    if (cnt > 128 || (cnt > 0 && reg_data == nullptr)) {
        return -1;
    }

    uint8_t frame[4 + 128];
    frame[0] = BNO055_UART_START_BYTE;
    frame[1] = BNO055_UART_WRITE_CMD;
    frame[2] = reg_addr;
    frame[3] = cnt;

    if (cnt > 0) {
        memcpy(&frame[4], reg_data, cnt);
    }

    flushBnoRx();
    Serial2.write(frame, 4 + cnt);
    Serial2.flush();

    uint8_t header = 0;
    uint8_t status = 0;
    if (!readByteTimeout(&header) || !readByteTimeout(&status)) {
        Serial.println("[IMU] UART write timeout");
        return -1;
    }

    if (header != BNO055_UART_RESP_WRITE || status != BNO055_UART_WRITE_SUCCESS) {
        Serial.printf("[IMU] UART write failed: header=0x%02X status=0x%02X\n", header, status);
        return -1;
    }

    return 0;
}

static int8_t bno_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    (void)dev_addr;

    if (cnt == 0 || reg_data == nullptr) {
        return -1;
    }

    const uint8_t frame[4] = {
        BNO055_UART_START_BYTE,
        BNO055_UART_READ_CMD,
        reg_addr,
        cnt
    };

    flushBnoRx();
    Serial2.write(frame, sizeof(frame));
    Serial2.flush();

    uint8_t header = 0;
    if (!readByteTimeout(&header)) {
        Serial.println("[IMU] UART read timeout waiting for header");
        return -1;
    }

    if (header == BNO055_UART_RESP_WRITE) {
        uint8_t status = 0;
        if (readByteTimeout(&status)) {
            Serial.printf("[IMU] UART read error status=0x%02X\n", status);
        }
        return -1;
    }

    if (header != BNO055_UART_RESP_READ) {
        Serial.printf("[IMU] UART read invalid header=0x%02X\n", header);
        return -1;
    }

    uint8_t responseLen = 0;
    if (!readByteTimeout(&responseLen)) {
        Serial.println("[IMU] UART read timeout waiting for length");
        return -1;
    }

    if (responseLen != cnt) {
        Serial.printf("[IMU] UART read length mismatch: expected=%u actual=%u\n", cnt, responseLen);
        for (uint8_t i = 0; i < responseLen; ++i) {
            uint8_t discard = 0;
            if (!readByteTimeout(&discard)) {
                break;
            }
        }
        return -1;
    }

    for (uint8_t i = 0; i < cnt; ++i) {
        if (!readByteTimeout(&reg_data[i])) {
            Serial.printf("[IMU] UART read timeout at byte %u\n", i);
            return -1;
        }
    }

    return 0;
}

static void bno_delay_msec(BNO055_MDELAY_DATA_TYPE msec)
{
    vTaskDelay(pdMS_TO_TICKS(static_cast<uint32_t>(msec)));
}

static void hardwareResetBno055()
{
    digitalWrite(BNO055_RESET_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(BNO055_RESET_LOW_MS));
    digitalWrite(BNO055_RESET_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(BNO055_BOOT_DELAY_MS));
    flushBnoRx();
}

static bool configureBno055Api()
{
    memset(&g_bno055, 0, sizeof(g_bno055));
    g_bno055.dev_addr = 0;
    g_bno055.bus_read = bno_read;
    g_bno055.bus_write = bno_write;
    g_bno055.delay_msec = bno_delay_msec;

    int8_t result = bno055_init(&g_bno055);
    if (result != BNO055_SUCCESS) {
        Serial.printf("[IMU] bno055_init failed: %d\n", result);
        return false;
    }

    result = bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
    if (result != BNO055_SUCCESS) {
        Serial.printf("[IMU] bno055_set_power_mode failed: %d\n", result);
        return false;
    }

    result = bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
    if (result != BNO055_SUCCESS) {
        Serial.printf("[IMU] bno055_set_operation_mode(NDOF) failed: %d\n", result);
        return false;
    }

    return true;
}

void TaskIMU(void *pvParameters)
{
    (void)pvParameters;

    pinMode(BNO055_RESET_PIN, OUTPUT);
    digitalWrite(BNO055_RESET_PIN, HIGH);
    Serial2.begin(BNO055_BAUD, SERIAL_8N1, BNO055_RX_PIN, BNO055_TX_PIN);
    flushBnoRx();

    enum
    {
        STATE_INIT,
        STATE_RUN,
        STATE_ERROR
    } state = STATE_INIT;

    TickType_t lastWakeTime = xTaskGetTickCount();
    uint32_t errorBackoffMs = 1000;

    for (;;)
    {
        switch (state)
        {
        case STATE_INIT:
            hardwareResetBno055();
            if (configureBno055Api())
            {
                Serial.println("[IMU] Initialized successfully");
                errorBackoffMs = 1000;
                lastWakeTime = xTaskGetTickCount();
                state = STATE_RUN;
            }
            else
            {
                Serial.println("[IMU] Initialization failed");
                state = STATE_ERROR;
            }
            break;

        case STATE_RUN:
        {
            struct bno055_quaternion_t quat;
            int8_t result = bno055_read_quaternion_wxyz(&quat);
            if (result != BNO055_SUCCESS)
            {
                Serial.printf("[IMU] bno055_read_quaternion_wxyz failed: %d\n", result);
                state = STATE_ERROR;
                break;
            }

            IMUSample_t sample;
            sample.w = static_cast<float>(quat.w) * BNO055_QUAT_SCALE;
            sample.x = static_cast<float>(quat.x) * BNO055_QUAT_SCALE;
            sample.y = static_cast<float>(quat.y) * BNO055_QUAT_SCALE;
            sample.z = static_cast<float>(quat.z) * BNO055_QUAT_SCALE;
            sample.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            xQueueOverwrite(imuQueue, &sample);
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(IMU_TASK_PERIOD_MS));
            break;
        }

        case STATE_ERROR:
            Serial.printf("[IMU] Backing off for %lu ms before reset/re-init\n",
                          static_cast<unsigned long>(errorBackoffMs));
            vTaskDelay(pdMS_TO_TICKS(errorBackoffMs));
            if (errorBackoffMs < 8000) {
                errorBackoffMs = (errorBackoffMs <= 4000) ? (errorBackoffMs * 2) : 8000;
            }
            state = STATE_INIT;
            break;
        }
    }
}
