#include <Arduino.h>
#include "tasks/baro_task.h"
#include <SPI.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ============================================================================
// Konstanten
// ============================================================================
const float DroneBaro::SEA_LEVEL_PRESSURE = 101325.0f;

// SPI Konfiguration für DPS368
const SPISettings DPS368_SPI_SETTINGS(10000000, MSBFIRST, SPI_MODE3);

// Hardware-Pins für Flight Controller
const int BARO_CS_PIN  = 37;  // Chip Select
const int BARO_INT_PIN = 34;  // Interrupt Pin
const int BARO_CLK_PIN = 36;  // Clock
const int BARO_MOSI_PIN = 35; // MOSI
const int BARO_MISO_PIN = 37; // MISO (wenn verwendet, sonst 3-Wire)

// FreeRTOS Task Parameter
const uint16_t BARO_TASK_STACK_SIZE = 4096;  // Stack Größe
const int BARO_TASK_PRIORITY = 5;            // Priority Level
const uint32_t BARO_UPDATE_RATE_MS = 50;     // 50ms = ~20Hz Update Rate

// Globale DroneBaro Instanz für Interrupt Handler
static DroneBaro *g_barometer = nullptr;

// ============================================================================
// DroneBaro Klassenmethoden
// ============================================================================

DroneBaro::DroneBaro(int csPin, int intPin, SPIClass &spiBus)
    : _csPin(csPin), _intPin(intPin), _spi(spiBus)
{
    memset(&_calib, 0, sizeof(_calib));
    memset(&_data, 0, sizeof(_data));
    _pressureOS = DPS368_OS_16X;
    _pressureRate = DPS368_RATE_64HZ;
}

// ============================================================================
// Initialisierung - State Machine nach Datenblatt
// ============================================================================
bool DroneBaro::init(
    DPS368_SamplingRate pressureRate,
    DPS368_Oversampling pressureOS,
    DPS368_SamplingRate tempRate,
    DPS368_Oversampling tempOS,
    DPS368_MeasurementMode mode)
{
    _pressureRate = pressureRate;
    _pressureOS = pressureOS;

    // Speichere globalen Zeiger für ISR
    g_barometer = this;

    // 1. GPIO Setup
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    if (_intPin != -1) {
        pinMode(_intPin, INPUT);
    }

    delay(10); // Stabilisierungsdelay

    // 2. Soft Reset durchführen
    if (!softReset()) {
        Serial.println("[BARO] ERROR: Soft Reset failed");
        return false;
    }

    // 3. Warte auf Sensor Ready
    if (!waitForReady(DPS368_SENSOR_RDY)) {
        Serial.println("[BARO] ERROR: Sensor not ready");
        return false;
    }

    // 4. Warte auf Koeffizient Ready
    if (!waitForReady(DPS368_COEF_RDY)) {
        Serial.println("[BARO] ERROR: Coefficients not ready");
        return false;
    }

    // 5. Lese Kalibrierungskoeffizienten
    if (!readCalibrationCoefficients()) {
        Serial.println("[BARO] ERROR: Failed to read calibration");
        return false;
    }

    // 6. Konfiguriere Register
    if (!configureRegisters(pressureRate, pressureOS, tempRate, tempOS, mode)) {
        Serial.println("[BARO] ERROR: Failed to configure");
        return false;
    }

    // 7. Richte Interrupt ein (optional)
    if (_intPin != -1) {
        //TODO: attachInterrupt(digitalPinToInterrupt(_intPin), isrHandler, FALLING);
    }

    Serial.println("[BARO] Initialized successfully");
    printCalibrationData();
    return true;
}

// ============================================================================
// Soft Reset - Initialisierungsphase
// ============================================================================
bool DroneBaro::softReset()
{
    // Schreibe Reset Befehl: 0b1001 = 0x09
    writeReg(DPS368_REG_RESET, 0x09);
    
    // Warte 10ms für Reset
    delay(10);
    
    return true;
}

// ============================================================================
// Wait for Ready - Polle Status Register mit Timeout
// ============================================================================
bool DroneBaro::waitForReady(uint8_t readyBits, uint32_t timeoutMs)
{
    uint32_t startTime = millis();
    
    while ((millis() - startTime) < timeoutMs) {
        uint8_t measCfg = readReg(DPS368_REG_MEAS_CFG);
        
        if ((measCfg & readyBits) == readyBits) {
            return true;
        }
        
        delay(1); // Kurzes Delay um CPU zu entlasten
    }
    
    return false;
}

// ============================================================================
// Lese Kalibrierungskoeffizienten aus PROM
// ============================================================================
bool DroneBaro::readCalibrationCoefficients()
{
    // Lese alle 18 Bytes Koeffizienten (Register 0x10 - 0x21)
    uint8_t buffer[18];
    readRegs(DPS368_REG_COEF_START, buffer, 18);
    
    // Parse die gepackten Koeffizienten
    parseCalibrationBytes(buffer);
    
    return true;
}

// ============================================================================
// Parse Kalibrierungsbytes - Zweierkomplementbehandlung
// ============================================================================
void DroneBaro::parseCalibrationBytes(const uint8_t *buffer)
{
    // Nach Datenblatt Tabelle 18: Gepackte Koeffizienten
    // Buffer Layout (18 Bytes):
    // c00: Bits 19:0 (Bytes 0[7:0], 1[7:4])
    // c10: Bits 19:0 (Bytes 1[3:0], 2[7:0])
    // c20: Bits 19:0 (Bytes 3[7:0], 4[7:4])
    // c30: Bits 19:0 (Bytes 4[3:0], 5[7:0])
    // c01: Bits 15:0 (Bytes 6[7:0], 7[7:0])
    // c11: Bits 19:0 (Bytes 8[7:0], 9[7:4])
    // c21: Bits 19:0 (Bytes 9[3:0], 10[7:0])
    // c0:  Bits 11:0 (Bytes 11[7:0], 12[7:4])
    // c1:  Bits 11:0 (Bytes 12[3:0], 13[7:0])

    // TODO: Implementiere korrekte Bit-Packing Extraktion und Sign Extension
    // Für jetzt: Vereinfachte Version (Datenblatt genau konsultieren!)
    
    // c00 (20-Bit, signed)
    int32_t tmp = ((int32_t)buffer[0] << 12) | ((int32_t)buffer[1] << 4);
    _calib.c00 = (tmp & 0x80000) ? (tmp - 0x100000) : tmp;

    // c10 (20-Bit, signed)
    tmp = ((int32_t)buffer[1] << 16) | ((int32_t)buffer[2] << 8);
    _calib.c10 = (tmp & 0x80000) ? (tmp - 0x100000) : tmp;

    // c20 (20-Bit, signed)
    tmp = ((int32_t)buffer[3] << 12) | ((int32_t)buffer[4] << 4);
    _calib.c20 = (tmp & 0x80000) ? (tmp - 0x100000) : tmp;

    // c30 (20-Bit, signed)
    tmp = ((int32_t)buffer[4] << 16) | ((int32_t)buffer[5] << 8);
    _calib.c30 = (tmp & 0x80000) ? (tmp - 0x100000) : tmp;

    // c01 (16-Bit, signed)
    _calib.c01 = ((int32_t)buffer[6] << 8) | buffer[7];
    if (_calib.c01 & 0x8000) _calib.c01 -= 0x10000;

    // c11 (20-Bit, signed)
    tmp = ((int32_t)buffer[8] << 12) | ((int32_t)buffer[9] << 4);
    _calib.c11 = (tmp & 0x80000) ? (tmp - 0x100000) : tmp;

    // c21 (20-Bit, signed)
    tmp = ((int32_t)buffer[9] << 16) | ((int32_t)buffer[10] << 8);
    _calib.c21 = (tmp & 0x80000) ? (tmp - 0x100000) : tmp;

    // c0 (12-Bit, signed)
    tmp = ((int32_t)buffer[11] << 4) | ((int32_t)buffer[12] >> 4);
    _calib.c0 = (tmp & 0x800) ? (tmp - 0x1000) : tmp;

    // c1 (12-Bit, signed)
    tmp = ((int32_t)(buffer[12] & 0x0F) << 8) | buffer[13];
    _calib.c1 = (tmp & 0x800) ? (tmp - 0x1000) : tmp;

    // TODO: Validiere Koeffiziente (z.B. nicht alle 0)
}

// ============================================================================
// Konfiguriere Register nach Vorgaben
// ============================================================================
bool DroneBaro::configureRegisters(
    DPS368_SamplingRate pressureRate,
    DPS368_Oversampling pressureOS,
    DPS368_SamplingRate tempRate,
    DPS368_Oversampling tempOS,
    DPS368_MeasurementMode mode)
{
    // 1. PRS_CFG - Druck Sampling Rate & Oversampling
    // Bits 6:4 = Rate, Bits 3:0 = Oversampling
    uint8_t prsCfg = ((uint8_t)pressureRate << 4) | (uint8_t)pressureOS;
    writeReg(DPS368_REG_PRS_CFG, prsCfg);

    // 2. TMP_CFG - Temperatur Sampling Rate & Oversampling
    // Bit 7 = Temp Source (intern), Bits 6:4 = Rate, Bits 3:0 = Oversampling
    uint8_t tmpCfg = 0x80 | ((uint8_t)tempRate << 4) | (uint8_t)tempOS;
    writeReg(DPS368_REG_TMP_CFG, tmpCfg);

    // 3. CFG_REG - Shift-Bits und andere Einstellungen
    // Bit 7 = P_SHIFT (setzen wenn Oversampling > 8x)
    // Bit 3 = T_SHIFT (setzen wenn Oversampling > 8x)
    uint8_t cfgReg = 0x00;
    if (pressureOS > DPS368_OS_8X) {
        cfgReg |= 0x80;  // P_SHIFT setzen
    }
    if (tempOS > DPS368_OS_8X) {
        cfgReg |= 0x08;  // T_SHIFT setzen
    }
    // TODO: Andere Bits je nach Anforderung (FIFO, SPI-Mode, etc.)
    writeReg(DPS368_REG_CFG_REG, cfgReg);

    // 4. MEAS_CFG - Messmodus (Continuous/Command)
    writeReg(DPS368_REG_MEAS_CFG, (uint8_t)mode);

    // Warte kurz für Konfiguration
    delay(50);

    return true;
}

// ============================================================================
// Update - Hauptfunktion für Datenakquisition
// ============================================================================
void DroneBaro::update()
{
    // Wenn Interrupt aktiviert: Warte auf Flag
    // Wenn kein Interrupt: Polle Status Register
    
    if (_intPin == -1) {
        // Polling-Modus (wenn kein Interrupt-Pin vorhanden)
        uint8_t intStatus = readReg(DPS368_REG_INT_STS);
        
        // Prüfe ob Druck UND Temperatur bereit sind
        if ((intStatus & (DPS368_PRS_RDY | DPS368_TMP_RDY)) == 
            (DPS368_PRS_RDY | DPS368_TMP_RDY)) {
            if (readMeasurement()) {
                calculateAltitude();
                _dataReady = true;
            }
        }
    } else {
        // Interrupt-Modus (wenn _dataReady durch ISR gesetzt)
        if (_dataReady) {
            if (readMeasurement()) {
                calculateAltitude();
            }
            _dataReady = false;
        }
    }
}

// ============================================================================
// Lese Rohwerte und berechne Druck & Temperatur
// ============================================================================
bool DroneBaro::readMeasurement()
{
    // Lese 6 Bytes: 3x Druck, 3x Temperatur
    uint8_t buffer[6];
    readRegs(DPS368_REG_PRS_B2, buffer, 6);

    // Druck: 24-Bit, Two's Complement
    int32_t raw_p = ((int32_t)buffer[0] << 16) |
                    ((int32_t)buffer[1] << 8) |
                    ((int32_t)buffer[2]);
    if (raw_p & 0x800000) raw_p -= 0x1000000;  // Sign Extension

    // Temperatur: 24-Bit, Two's Complement
    int32_t raw_t = ((int32_t)buffer[3] << 16) |
                    ((int32_t)buffer[4] << 8) |
                    ((int32_t)buffer[5]);
    if (raw_t & 0x800000) raw_t -= 0x1000000;  // Sign Extension

    // Ermittle Scaling Factor basierend auf Oversampling
    int32_t scalingFactorP = DPS368_SCALE_FACTORS[_pressureOS];

    // Temperatur-Skalierung: Nutze Pressure OS (TODO: Könnte separater sein)
    int32_t scalingFactorT = DPS368_SCALE_FACTORS[DPS368_OS_2X]; // Typischerweise niedriger OS

    // Berechne skalierte Werte
    float p_sc = (float)raw_p / (float)scalingFactorP;
    float t_sc = (float)raw_t / (float)scalingFactorT;

    // ========================================================================
    // Kompensationsformeln nach Datenblatt
    // ========================================================================

    // 1. Temperaturberechnung
    _data.temperature_degc = (float)_calib.c0 * 0.5f + (float)_calib.c1 * t_sc;

    // 2. Druckberechnung (polynom)
    float comp_press = (float)_calib.c00
                      + p_sc * ((float)_calib.c10 + p_sc * ((float)_calib.c20 + p_sc * (float)_calib.c30))
                      + t_sc * (float)_calib.c01
                      + t_sc * p_sc * ((float)_calib.c11 + p_sc * (float)_calib.c21);

    _data.pressure_pa = comp_press;

    return true;
}

// ============================================================================
// Berechne Höhe aus Druck und Referenzdruck
// ============================================================================
void DroneBaro::calculateAltitude()
{
    // Barometrische Höhenformel
    // h = 44330 * (1 - (P/P0)^(1/5.255))
    float pressureRatio = _data.pressure_pa / SEA_LEVEL_PRESSURE;
    
    if (pressureRatio > 0.0f) {
        _data.altitude_m = 44330.0f * (1.0f - powf(pressureRatio, 0.1902949f));
    } else {
        _data.altitude_m = 0.0f; // Ungültig
    }
}

// ============================================================================
// Low-Level SPI Operationen
// ============================================================================

void DroneBaro::writeReg(uint8_t reg, uint8_t value)
{
    _spi.beginTransaction(DPS368_SPI_SETTINGS);
    digitalWrite(_csPin, LOW);
    
    // Register Address mit Write Bit (MSB=0)
    _spi.transfer(reg & 0x7F);
    _spi.transfer(value);
    
    digitalWrite(_csPin, HIGH);
    _spi.endTransaction();
}

uint8_t DroneBaro::readReg(uint8_t reg)
{
    _spi.beginTransaction(DPS368_SPI_SETTINGS);
    digitalWrite(_csPin, LOW);
    
    // Register Address mit Read Bit (MSB=1)
    _spi.transfer(reg | 0x80);
    uint8_t result = _spi.transfer(0x00);
    
    digitalWrite(_csPin, HIGH);
    _spi.endTransaction();
    
    return result;
}

void DroneBaro::readRegs(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    _spi.beginTransaction(DPS368_SPI_SETTINGS);
    digitalWrite(_csPin, LOW);
    
    // Register Address mit Read Bit (MSB=1)
    _spi.transfer(reg | 0x80);
    
    // Lese Bytes nacheinander
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = _spi.transfer(0x00);
    }
    
    digitalWrite(_csPin, HIGH);
    _spi.endTransaction();
}

// ============================================================================
// Interrupt Service Routine
// ============================================================================
void IRAM_ATTR DroneBaro::isrHandler(void *arg)
{
    if (g_barometer != nullptr) {
        g_barometer->_dataReady = true;
    }
}

// ============================================================================
// Debug Funktion
// ============================================================================
void DroneBaro::printCalibrationData() const
{
    Serial.println("[BARO] Calibration Coefficients:");
    Serial.printf("  c00 = %ld\n", _calib.c00);
    Serial.printf("  c10 = %ld\n", _calib.c10);
    Serial.printf("  c20 = %ld\n", _calib.c20);
    Serial.printf("  c30 = %ld\n", _calib.c30);
    Serial.printf("  c01 = %ld\n", _calib.c01);
    Serial.printf("  c11 = %ld\n", _calib.c11);
    Serial.printf("  c21 = %ld\n", _calib.c21);
    Serial.printf("  c0  = %ld\n", _calib.c0);
    Serial.printf("  c1  = %ld\n", _calib.c1);
}

// ============================================================================
// FreeRTOS Task
// ============================================================================
void TaskBaro(void *pvParameters)
{
    // Erstelle DroneBaro Instanz
    DroneBaro barometer(BARO_CS_PIN, BARO_INT_PIN, SPI);

    // Initialisiere mit Standard-Einstellungen
    // - 64 Hz Druckmessrate, 16x Oversampling (guter Kompromiss)
    // - 4 Hz Temperaturmessrate, 2x Oversampling (Temp ändert sich langsam)
    // - Continuous Modus (Druck + Temperatur)
    if (!barometer.init(
        DPS368_RATE_64HZ,      // Druckmessrate
        DPS368_OS_16X,         // Druck Oversampling
        DPS368_RATE_4HZ,       // Temperaturmessrate
        DPS368_OS_2X,          // Temperatur Oversampling
        DPS368_MODE_CONT_BOTH  // Kontinuierlich beide Messungen
    )) {
        Serial.println("[TaskBaro] Initialization failed!");
        vTaskDelete(nullptr);
        return;
    }

    // Zeitsteuerung: Präzises FreeRTOS Timing
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(BARO_UPDATE_RATE_MS);

    Serial.println("[TaskBaro] Task started, 20Hz update rate");

    // Hauptschleife
    for (;;) {
        // Update Barometer-Daten
        barometer.update();

        // Debug Output (TODO: nur während Entwicklung, später auskommentieren)
        if (barometer.isDataReady()) {
            const DPS368_Data &data = barometer.getData();
            Serial.printf("[BARO] P=%.0f Pa, T=%.1f C, Alt=%.1f m\n",
                         data.pressure_pa,
                         data.temperature_degc,
                         data.altitude_m);
            
            // TODO: Schreibe Daten in globale Variable oder Queue für andere Tasks
            // Beispiel:
            // sensorData.barometer.altitude = barometer.getAltitude();
            // sensorData.barometer.pressure = barometer.getPressure();
            // sensorData.barometer.temperature = barometer.getTemperature();
        }

        // Präzises Warten (konstante Frequenz unabhängig von Rechenzeit)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}