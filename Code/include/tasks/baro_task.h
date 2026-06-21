#ifndef BARO_TASK_H
#define BARO_TASK_H

#include <Arduino.h>
#include <SPI.h>
#include <Dps310.h>
#include <cmath>

// Compatibility enums for existing DroneBaro callers. The numeric values match
// the Infineon DPS3xx library's rate/oversampling fields.
enum DPS368_Oversampling {
    DPS368_OS_1X     = 0x00,
    DPS368_OS_2X     = 0x01,
    DPS368_OS_4X     = 0x02,
    DPS368_OS_8X     = 0x03,
    DPS368_OS_16X    = 0x04,
    DPS368_OS_32X    = 0x05,
    DPS368_OS_64X    = 0x06,
    DPS368_OS_128X   = 0x07
};

enum DPS368_SamplingRate {
    DPS368_RATE_1HZ    = 0x00,
    DPS368_RATE_2HZ    = 0x01,
    DPS368_RATE_4HZ    = 0x02,
    DPS368_RATE_8HZ    = 0x03,
    DPS368_RATE_16HZ   = 0x04,
    DPS368_RATE_32HZ   = 0x05,
    DPS368_RATE_64HZ   = 0x06,
    DPS368_RATE_128HZ  = 0x07
};

enum DPS368_MeasurementMode {
    DPS368_MODE_IDLE          = 0x00,
    DPS368_MODE_CMD_PRESSURE  = 0x01,
    DPS368_MODE_CMD_TEMP      = 0x02,
    DPS368_MODE_CMD_BOTH      = 0x03,
    DPS368_MODE_CONT_PRESSURE = 0x04,
    DPS368_MODE_CONT_TEMP     = 0x05,
    DPS368_MODE_CONT_BOTH     = 0x06
};

struct DPS368_Data {
    float pressure_pa;
    float temperature_degc;
    float altitude_m;
};

class DroneBaro {
public:
    DroneBaro(int csPin, int intPin, SPIClass &spiBus);

    bool init(
        DPS368_SamplingRate pressureRate = DPS368_RATE_64HZ,
        DPS368_Oversampling pressureOS = DPS368_OS_16X,
        DPS368_SamplingRate tempRate = DPS368_RATE_4HZ,
        DPS368_Oversampling tempOS = DPS368_OS_2X,
        DPS368_MeasurementMode mode = DPS368_MODE_CONT_BOTH
    );

    void update();

    float getAltitude() const { return _data.altitude_m; }
    float getPressure() const { return _data.pressure_pa; }
    float getTemperature() const { return _data.temperature_degc; }
    const DPS368_Data &getData() const { return _data; }
    bool isDataReady() const { return _dataReady; }

    void printCalibrationData() const;

private:
    int _csPin, _intPin;
    SPIClass &_spi;
    Dps310 _sensor;

    DPS368_Data _data;
    bool _dataReady = false;

    DPS368_Oversampling _pressureOS;
    DPS368_SamplingRate _pressureRate;
    DPS368_Oversampling _tempOS;
    DPS368_SamplingRate _tempRate;

    static const float SEA_LEVEL_PRESSURE;

    bool startMeasurements(DPS368_MeasurementMode mode);
    bool readMeasurement();
    void calculateAltitude();
};

void TaskBaro(void *pvParameters);

#endif // BARO_TASK_H
