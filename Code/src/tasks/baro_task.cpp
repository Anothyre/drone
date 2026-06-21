#include <Arduino.h>
#include "tasks/baro_task.h"
#include "shared.h"
#include "data_structures.h"
#include <SPI.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const float DroneBaro::SEA_LEVEL_PRESSURE = 101325.0f;

const int BARO_CS_PIN  = 37;  // Chip Select
const int BARO_INT_PIN = 34;  // Interrupt Pin
const int BARO_CLK_PIN = 36;  // Clock
const int BARO_MOSI_PIN = 35; // MOSI
const int BARO_MISO_PIN = 37; // MISO

const uint16_t BARO_TASK_STACK_SIZE = 4096;
const int BARO_TASK_PRIORITY = 5;
const uint32_t BARO_UPDATE_RATE_MS = 50;

DroneBaro::DroneBaro(int csPin, int intPin, SPIClass &spiBus)
    : _csPin(csPin), _intPin(intPin), _spi(spiBus)
{
    memset(&_data, 0, sizeof(_data));
    _pressureOS = DPS368_OS_16X;
    _pressureRate = DPS368_RATE_64HZ;
    _tempOS = DPS368_OS_2X;
    _tempRate = DPS368_RATE_4HZ;
}

bool DroneBaro::init(
    DPS368_SamplingRate pressureRate,
    DPS368_Oversampling pressureOS,
    DPS368_SamplingRate tempRate,
    DPS368_Oversampling tempOS,
    DPS368_MeasurementMode mode)
{
    _pressureRate = pressureRate;
    _pressureOS = pressureOS;
    _tempRate = tempRate;
    _tempOS = tempOS;
    _dataReady = false;

    if (BARO_CS_PIN == BARO_MISO_PIN) {
        Serial.println("[BARO] ERROR: BARO_CS_PIN and BARO_MISO_PIN are both set to GPIO37.");
        Serial.println("[BARO] Fix the wiring/pin config before SPI barometer access can work.");
        return false;
    }

    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    if (_intPin != -1) {
        pinMode(_intPin, INPUT);
    }

    _spi.begin(BARO_CLK_PIN, BARO_MISO_PIN, BARO_MOSI_PIN, _csPin);
    _sensor.begin(_spi, _csPin);

    if (!startMeasurements(mode)) {
        Serial.println("[BARO] ERROR: Failed to start measurements");
        return false;
    }

    Serial.printf("[BARO] Initialized successfully (Product ID 0x%02X, Revision 0x%02X)\n",
                  _sensor.getProductId(),
                  _sensor.getRevisionId());
    return true;
}

bool DroneBaro::startMeasurements(DPS368_MeasurementMode mode)
{
    int16_t result = -1;

    switch (mode) {
        case DPS368_MODE_CONT_BOTH:
            result = _sensor.startMeasureBothCont(
                (uint8_t)_tempRate,
                (uint8_t)_tempOS,
                (uint8_t)_pressureRate,
                (uint8_t)_pressureOS);
            break;
        case DPS368_MODE_CONT_PRESSURE:
            result = _sensor.startMeasurePressureCont((uint8_t)_pressureRate, (uint8_t)_pressureOS);
            break;
        case DPS368_MODE_CONT_TEMP:
            result = _sensor.startMeasureTempCont((uint8_t)_tempRate, (uint8_t)_tempOS);
            break;
        case DPS368_MODE_IDLE:
            result = _sensor.standby();
            break;
        default:
            Serial.println("[BARO] ERROR: Command measurement modes are not used by TaskBaro");
            return false;
    }

    if (result != 0) {
        Serial.printf("[BARO] ERROR: Infineon library returned %d while starting measurements\n", result);
        return false;
    }

    return mode != DPS368_MODE_IDLE;
}

void DroneBaro::update()
{
    _dataReady = false;

    if (readMeasurement()) {
        calculateAltitude();
        _dataReady = true;

        BaroData baroMsg = {
            _data.pressure_pa,
            _data.temperature_degc,
            _data.altitude_m
        };
        xQueueOverwrite(baroQueue, &baroMsg);
    }
}

bool DroneBaro::readMeasurement()
{
    float tempBuffer[8];
    float pressureBuffer[8];
    uint8_t tempCount = 8;
    uint8_t pressureCount = 8;

    int16_t result = _sensor.getContResults(tempBuffer, tempCount, pressureBuffer, pressureCount);
    if (result != 0) {
        Serial.printf("[BARO] WARN: Infineon library returned %d while reading\n", result);
        return false;
    }

    if (tempCount == 0 || pressureCount == 0) {
        return false;
    }

    _data.temperature_degc = tempBuffer[tempCount - 1];
    _data.pressure_pa = pressureBuffer[pressureCount - 1];
    return true;
}

void DroneBaro::calculateAltitude()
{
    float pressureRatio = _data.pressure_pa / SEA_LEVEL_PRESSURE;

    if (pressureRatio > 0.0f) {
        _data.altitude_m = 44330.0f * (1.0f - powf(pressureRatio, 0.1902949f));
    } else {
        _data.altitude_m = 0.0f;
    }
}

void DroneBaro::printCalibrationData() const
{
    Serial.println("[BARO] Calibration coefficients are managed by the Infineon DPS310 library.");
}

void TaskBaro(void *pvParameters)
{
    DroneBaro barometer(BARO_CS_PIN, BARO_INT_PIN, SPI);

    if (!barometer.init(
        DPS368_RATE_64HZ,
        DPS368_OS_16X,
        DPS368_RATE_4HZ,
        DPS368_OS_2X,
        DPS368_MODE_CONT_BOTH
    )) {
        Serial.println("[TaskBaro] Initialization failed!");
        vTaskDelete(nullptr);
        return;
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(BARO_UPDATE_RATE_MS);

    Serial.println("[TaskBaro] Task started, 20Hz update rate");

    for (;;) {
        barometer.update();

        if (barometer.isDataReady()) {
            const DPS368_Data &data = barometer.getData();
            Serial.printf("[BARO] P=%.0f Pa, T=%.1f C, Alt=%.1f m\n",
                          data.pressure_pa,
                          data.temperature_degc,
                          data.altitude_m);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
