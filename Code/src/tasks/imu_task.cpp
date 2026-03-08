#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks/imu_task.h"
#include "shared.h" // access imuQueue
#include <cstring>

/* =========================================================
   BNO055 UART Communication
   =========================================================
   TODO: 
   THERE IS CURRENTLY AN ERROR WHEN USING THE RESET BUTTON
   SIMILARLY WHEN THE IMU HAS AN ERROR THE TASK RESETS THE IMU BUT THEN IMMEDIATELY FAILS AGAIN.
   NEED TO INVESTIGATE FURTHER.

    Possible causes

        UART timing/flush race after hardware reset: sensor boot messages or stale bytes are misinterpreted as protocol headers.
        Timeout too aggressive: readByteTimeout(30 ms) may expire or cut off slow responses after reset.
        Reset timing too short: BNO055 may need longer post-reset boot time than BNO055_DELAY_RESET_MS used.
        Repeated immediate re-init: task moves to STATE_INIT quickly (error loop), resetting the sensor before it finishes booting.
        Serial2.end()/begin() misuse: reinitializing UART repeatedly or changing pin state can create transient RX/TX issues.
        Reset-pin wiring or shared reset button: manual presses may conflict with code-driven reset (active-low vs pull-ups).
        Blocking delays vs FreeRTOS timing: mixing delay() and vTaskDelay can change scheduling and timing expectations.
        Protocol framing mismatch: partial/shifted frames on the wire lead to invalid headers (0xEE / unexpected values).
    
    Suggested quick fixes to try

        Increase read timeout (e.g., 100–200 ms) in readByteTimeout to tolerate boot chatter.
        After reset, wait longer and discard UART for a longer period (e.g., 500–1000 ms) before sending commands.
        Add exponential backoff on repeated init failures (avoid resetting every 100 ms).
        Log raw first few bytes on unexpected header to see what the sensor actually sends.
        Replace delay(1) in readByteTimeout with a FreeRTOS-friendly yield or vTaskDelay(1) if needed.
        Ensure the reset button wiring and pinMode (INPUT_PULLUP vs external pull-down) match active-low logic.
        On error, avoid calling Serial2.end() too often; instead reinit Serial2 only when attempting full re-init.
        Verify BNO055 timing requirements in datasheet and match delays for boot, mode switching, and unit writes.
        Add a guarded state: after reset, poll chip ID repeatedly with small delays until stable before proceeding.
*/



/* =========================================================
   Low-Level UART
   ========================================================= */

// helper that reads single byte with timeout from Serial2
static bool readByteTimeout(uint8_t *out)
{
    unsigned long start = millis();
    while (Serial2.available() == 0)
    {
        if (millis() - start >= 30)
            return false;
        delay(1);
    }
    *out = (uint8_t)Serial2.read();
    return true;
}

static bool bno_write(uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t frame[4 + 128];

    frame[0] = BNO055_START_BYTE;
    frame[1] = BNO055_WRITE_CMD;
    frame[2] = reg;
    frame[3] = len;

    if (len > 0 && data != nullptr)
        memcpy(&frame[4], data, len);

    // clear any stale input
    while (Serial2.available())
        Serial2.read();

    // send all bytes
    Serial2.write(frame, 4 + len);
    Serial2.flush();
    delay(2 * (4 + len)); // allow transmission and inter-byte gap

    // read response header
    uint8_t header;
    if (!readByteTimeout(&header))
    {
        Serial.println("bno_write: no response header");
        Serial.flush();
        return false;
    }

    if (header == BNO055_RESP_READ || header == BNO055_RESP_WRITE)
    {
        uint8_t status;
        if (!readByteTimeout(&status))
        {
            Serial.println("bno_write: response missing status");
            Serial.flush();
            return false;
        }
        if (header == BNO055_RESP_WRITE)
        {
            if (status == BNO055_WRITE_SUCCESS)
                return true;
            Serial.printf("bno_write: write failed status=0x%02X\n", status);
            Serial.flush();
        }
        else
        {
            Serial.printf("bno_write: unexpected read-ack status=0x%02X\n", status);
            Serial.flush();
        }
    }
    else
    {
        Serial.printf("bno_write: invalid response header 0x%02X\n", header);
        Serial.flush();
    }
    return false;
}

static bool bno_read(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t frame[4];

    frame[0] = BNO055_START_BYTE;
    frame[1] = BNO055_READ_CMD;
    frame[2] = reg;
    frame[3] = len;

    Serial.printf("Requesting read of %d bytes from reg 0x%02X.\n", len, reg);
    Serial.flush();

    // flush any RX
    while (Serial2.available())
        Serial2.read();

    // send command
    Serial2.write(frame, 4);
    Serial2.flush();
    delay(8);

    // read header
    uint8_t header;
    if (!readByteTimeout(&header))
    {
        Serial.println("Failed to receive read response header.");
        Serial.flush();
        return false;
    }

    if (header == BNO055_RESP_READ)
    {
        uint8_t resp_len;
        if (!readByteTimeout(&resp_len))
        {
            Serial.println("Failed to receive read response length.");
            Serial.flush();
            return false;
        }

        uint8_t to_read = resp_len;
        if (to_read > len)
            to_read = len;

        for (uint8_t i = 0; i < to_read; ++i)
        {
            if (!readByteTimeout(&data[i]))
            {
                Serial.printf("Failed to receive data byte %d\n", i);
                Serial.flush();
                return false;
            }
        }
        Serial.printf("Read %d bytes from reg 0x%02X.\n", to_read, reg);
        Serial.flush();
        return true;
    }
    else if (header == 0xEE)
    {
        uint8_t status;
        if (readByteTimeout(&status))
        {
            Serial.printf("bno_read: error status=0x%02X\n", status);
            Serial.flush();
        }
        return false;
    }
    else
    {
        Serial.printf("bno_read: invalid header 0x%02X\n", header);
        Serial.flush();
        return false;
    }
}

/* =========================================================
   Initialization
   ========================================================= */

static bool imu_init()
{
    // Prepare reset pin
    pinMode(BNO055_RESET_PIN, OUTPUT);
    digitalWrite(BNO055_RESET_PIN, HIGH);

    uint8_t id = 0;
    const int maxAttempts = 3;
    for (int attempt = 1; attempt <= maxAttempts; ++attempt)
    {
        Serial.printf("IMU init attempt %d\n", attempt);
        Serial.flush();

        // hardware reset: active-low pulse
        digitalWrite(BNO055_RESET_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(BNO055_RESET_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(BNO055_DELAY_RESET_MS));

        // (re)configure UART only after sensor has booted
        Serial2.end();
        Serial2.begin(BNO055_BAUD, SERIAL_8N1, BNO055_RX_PIN, BNO055_TX_PIN);
        delay(20);

        // flush any junk from UART after reset
        while (Serial2.available()){
            Serial2.read();
        }

        if (!bno_read(BNO055_REG_CHIP_ID, &id, 1))
        {
            Serial.println("Failed to read IMU chip ID.");
            Serial.flush();
        }
        else if (id != BNO055_CHIP_ID_VALUE)
        {
            Serial.printf("Unexpected IMU chip ID: 0x%02X\n", id);
            Serial.flush();
            id = 0; // force retry
        }

        if (id == BNO055_CHIP_ID_VALUE){
            break;
        }


        if (attempt < maxAttempts)
        {
            Serial.println("Retrying IMU init...");
            Serial.flush();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }

    if (id != BNO055_CHIP_ID_VALUE)
    {
        Serial.println("IMU initialization failed after multiple attempts.");
        Serial.flush();
        return false;
    }

    // set configuration mode
    uint8_t mode = BNO055_MODE_CONFIG;
    if (!bno_write(BNO055_REG_OPR_MODE, &mode, 1))
    {
        Serial.println("Failed to set IMU to config mode.");
        Serial.flush();
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(BNO055_DELAY_CONFIG_MS));

    // set units (m/s², rad/s, radians, °C, Windows)
    uint8_t units = BNO055_UNIT_SEL_DEFAULT;
    if (!bno_write(BNO055_REG_UNIT_SEL, &units, 1))
    {
        Serial.println("Failed to set IMU units.");
        Serial.flush();
        return false;
    }

    // set NDOF mode (fusion of all sensors)
    mode = BNO055_MODE_NDOF;
    if (!bno_write(BNO055_REG_OPR_MODE, &mode, 1))
    {
        Serial.println("Failed to set IMU to NDOF mode.");
        Serial.flush();
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(BNO055_DELAY_MODE_SWITCH_MS));

    return true;
}

/* =========================================================
   IMU TASK
   ========================================================= */

void TaskIMU(void *pvParameters)
{
    (void)pvParameters;

    TickType_t lastWakeTime = xTaskGetTickCount();

    enum
    {
        STATE_INIT,
        STATE_RUN,
        STATE_ERROR
    } state = STATE_INIT;

    for (;;)
    {
        switch (state)
        {
        case STATE_INIT:
            if (imu_init())
            {
                Serial.printf("Initialized IMU successfully.\n");
                state = STATE_RUN;
            }
            else
            {
                Serial.printf("Failed to initialize IMU.\n");
                state = STATE_ERROR;
            }
            break;

        case STATE_RUN:
        {
            uint8_t raw[8];

            if (!bno_read(BNO055_REG_QUAT_LSB, raw, 8))
            {
                Serial.printf("Failed to read IMU data.\n");
                state = STATE_ERROR;
                break;
            }

            int16_t w = (raw[1] << 8) | raw[0];
            int16_t x = (raw[3] << 8) | raw[2];
            int16_t y = (raw[5] << 8) | raw[4];
            int16_t z = (raw[7] << 8) | raw[6];

            IMUSample_t sample;

            sample.w = static_cast<float>(w) * BNO055_QUAT_SCALE;
            sample.x = static_cast<float>(x) * BNO055_QUAT_SCALE;
            sample.y = static_cast<float>(y) * BNO055_QUAT_SCALE;
            sample.z = static_cast<float>(z) * BNO055_QUAT_SCALE;
            sample.timestamp_ms =
                xTaskGetTickCount() * portTICK_PERIOD_MS;

            xQueueSend(imuQueue, &sample, 0);

            break;
        }

        case STATE_ERROR:
            vTaskDelay(pdMS_TO_TICKS(100));
            Serial2.end(); // ensure UART is closed in case of error
            state = STATE_INIT;
            break;
        }

        vTaskDelayUntil(&lastWakeTime,
                        IMU_TASK_PERIOD_MS / portTICK_PERIOD_MS);
    }
}
