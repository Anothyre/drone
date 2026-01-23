#include <Arduino.h>    
#include "Wire.h"
#include "tasks/baro_task.h"
#include <SPI.h>
#include <cmath>



//GEMINI DEMO - TODO: think about

// Konstanten für den DPS368 (16x Oversampling)//TODO: Anpassen je nach Oversampling
const int32_t SCALING_FACTOR = 253952;
const float SEA_LEVEL_PRESSURE = 101325.0f;



// Beispielhafte Implementierung der Task
void TaskBaro(void *pvParameters)
{
    // SETUP: Wird einmal beim Start der Task ausgeführt
    const int csPin = 5; // Dein CS Pin
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);

    // Initialisierung der Kalibrierungskoeffizienten (Platzhalter-Werte)
    // Diese müssen normalerweise einmalig aus dem Sensor-PROM gelesen werden
    int32_t c00 = 0, c10 = 0, c20 = 0, c30 = 0, c01 = 0, c11 = 0, c21 = 0, c0 = 0, c1 = 0;

    // Zeitsteuerung für 50Hz (20ms Periode)
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20);

    for (;;) 
    {
        // 1. SPI Burst Read
        uint8_t buffer[6];
        SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE3));
        digitalWrite(csPin, LOW);
        SPI.transfer(0x00 | 0x80); // Read Mode ab Register 0x00
        for(int i = 0; i < 6; i++) {
            buffer[i] = SPI.transfer(0x00);
        }
        digitalWrite(csPin, HIGH);
        SPI.endTransaction();

        // 2. Rohwerte (24-bit) & Sign Extension
        int32_t raw_p = (int32_t)buffer[0] << 16 | (int32_t)buffer[1] << 8 | (int32_t)buffer[2];
        int32_t raw_t = (int32_t)buffer[3] << 16 | (int32_t)buffer[4] << 8 | (int32_t)buffer[5];
        if (raw_p & 0x800000) raw_p -= 0x1000000;
        if (raw_t & 0x800000) raw_t -= 0x1000000;

        // 3. Berechnung
        float p_sc = (float)raw_p / SCALING_FACTOR;
        float t_sc = (float)raw_t / SCALING_FACTOR;

        float comp_press = (float)c00 
                         + p_sc * ((float)c10 + p_sc * ((float)c20 + p_sc * (float)c30)) 
                         + t_sc * (float)c01 
                         + t_sc * p_sc * ((float)c11 + p_sc * (float)c21);

        float altitude = 44330.0f * (1.0f - powf(comp_press / SEA_LEVEL_PRESSURE, 0.1902949f));

        // Hier könntest du die Daten in eine globale Variable oder Queue schreiben
        // Serial.println(altitude); 

        // 4. Präzises Warten (FreeRTOS)
        // vTaskDelayUntil sorgt für eine konstante Frequenz, unabhängig von der Rechenzeit
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}