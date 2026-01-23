


#ifndef BARO_TASK_H
#define BARO_TASK_H

#include <Arduino.h>
#include <SPI.h>



class DroneBaro {
public:
    DroneBaro(int csPin, int intPin, SPIClass &spiBus);
    bool init();
    void update(); 
    float getAltitude() { return _altitude; }

private:
    int _cs, _int;
    SPIClass &_spi;
    volatile bool _dataReady = false; 

    // Kalibrierkoeffizienten
    int32_t c0, c1, c00, c10, c20, c30, c01, c11, c21;
    
    float _altitude = 0.0f;
    const float SEA_LEVEL_PRESSURE = 101325.0f;
    const int32_t SCALING_FACTOR = 253952; // Für 16x Oversampling

    void readCalibration();
    void writeReg(uint8_t reg, uint8_t val);
    void readRegs(uint8_t reg, uint8_t *buf, uint8_t len);
    
    static void IRAM_ATTR isrHandler(void* arg);
};
// src/tasks/baro_task.h



// FreeRTOS Task Definition
void TaskBaro(void *pvParameters);

#endif
