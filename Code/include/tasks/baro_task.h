


#ifndef BARO_TASK_H
#define BARO_TASK_H

#include <Arduino.h>
#include <SPI.h>
#include <cmath>

// ============================================================================
// DPS368 Register Adressen
// ============================================================================
#define DPS368_REG_PRS_B2      0x00  // Druck MSB (Bits 23:16)
#define DPS368_REG_PRS_B1      0x01  // Druck LSB (Bits 15:8)
#define DPS368_REG_PRS_B0      0x02  // Druck XLSB (Bits 7:0)
#define DPS368_REG_TMP_B2      0x03  // Temperatur MSB
#define DPS368_REG_TMP_B1      0x04  // Temperatur LSB
#define DPS368_REG_TMP_B0      0x05  // Temperatur XLSB
#define DPS368_REG_PRS_CFG     0x06  // Druck-Konfiguration (Rate & Oversampling)
#define DPS368_REG_TMP_CFG     0x07  // Temp-Konfiguration (Rate & Oversampling)
#define DPS368_REG_MEAS_CFG    0x08  // Messmodus & Status Flags
#define DPS368_REG_CFG_REG     0x09  // Interrupts, FIFO, Shift-Bits, SPI-Mode
#define DPS368_REG_INT_STS     0x0A  // Interrupt Status Flags
#define DPS368_REG_FIFO_STS    0x0B  // FIFO Status
#define DPS368_REG_RESET       0x0C  // Soft Reset
#define DPS368_REG_COEF_START  0x10  // Kalibrierungs-Koeffizienten Start
#define DPS368_REG_COEF_END    0x21  // Kalibrierungs-Koeffizienten End (18 Bytes)

// Status Bits in MEAS_CFG
#define DPS368_SENSOR_RDY      0x40  // Sensor Ready (Bit 6)
#define DPS368_COEF_RDY        0x80  // Koeffizienten Ready (Bit 7)

// Status Bits in INT_STS
#define DPS368_PRS_RDY         0x02  // Pressure Ready (Bit 1)
#define DPS368_TMP_RDY         0x04  // Temperature Ready (Bit 2)

// ============================================================================
// DPS368 Konfigurationen
// ============================================================================

// Oversampling Modi - bestimmen Präzision und Messdauer
enum DPS368_Oversampling {
    DPS368_OS_1X     = 0x00,  // 1x   - Schnell (3.6ms), hohes Rauschen
    DPS368_OS_2X     = 0x01,  // 2x
    DPS368_OS_4X     = 0x02,  // 4x
    DPS368_OS_8X     = 0x03,  // 8x
    DPS368_OS_16X    = 0x04,  // 16x  - Guter Kompromiss (27.6ms)
    DPS368_OS_32X    = 0x05,  // 32x
    DPS368_OS_64X    = 0x06,  // 64x
    DPS368_OS_128X   = 0x07   // 128x - Höchste Präzision (~200ms)
};

// Sampling Rates [Hz] - bestimmt Aktualisierungsfrequenz
enum DPS368_SamplingRate {
    DPS368_RATE_1HZ    = 0x00,
    DPS368_RATE_2HZ    = 0x01,
    DPS368_RATE_4HZ    = 0x02,
    DPS368_RATE_8HZ    = 0x03,
    DPS368_RATE_16HZ   = 0x04,
    DPS368_RATE_32HZ   = 0x05,
    DPS368_RATE_64HZ   = 0x06,  // Gut für Drohnen
    DPS368_RATE_128HZ  = 0x07   // Maximale Rate
};

// Betriebsmodi (Scheduling)
enum DPS368_MeasurementMode {
    DPS368_MODE_IDLE          = 0x00,  // Sensor ausgeschaltet
    DPS368_MODE_CMD_PRESSURE  = 0x01,  // Single-Shot Druck
    DPS368_MODE_CMD_TEMP      = 0x02,  // Single-Shot Temperatur
    DPS368_MODE_CMD_BOTH      = 0x03,  // Single-Shot Beide
    DPS368_MODE_CONT_PRESSURE = 0x04,  // Kontinuierlich Druck
    DPS368_MODE_CONT_TEMP     = 0x05,  // Kontinuierlich Temperatur
    DPS368_MODE_CONT_BOTH     = 0x06   // Kontinuierlich Beide (empfohlen)
};

// Scaling Factors Lookup-Table (abhängig von Oversampling)
// Nach Datenblatt Tabelle 9
static const int32_t DPS368_SCALE_FACTORS[] = {
    524288,    // 1x
    1048576,   // 2x
    2097152,   // 4x
    4194304,   // 8x
    8388608,   // 16x (253952 war falsch skaliert - das ist nur für Teilformel)
    16777216,  // 32x
    33554432,  // 64x
    67108864   // 128x
};

// ============================================================================
// DPS368 Kalibrierungs-Struktur
// ============================================================================
struct DPS368_Calib {
    // 20-Bit signed coefficients (packed in register space)
    int32_t c00, c10, c20, c30, c01, c11, c21;
    // 16-Bit signed coefficients
    int32_t c0, c1;
};

// ============================================================================
// DPS368 Datenstruktur
// ============================================================================
struct DPS368_Data {
    float pressure_pa;      // Druck in Pascal
    float temperature_degc; // Temperatur in Grad Celsius
    float altitude_m;       // Höhe in Meter
};

// ============================================================================
// DroneBaro Klasse - Hardware Abstraction Layer
// ============================================================================
class DroneBaro {
public:
    // Konstruktor
    DroneBaro(int csPin, int intPin, SPIClass &spiBus);
    
    // Initialisierung - muss vor update() aufgerufen werden
    bool init(
        DPS368_SamplingRate pressureRate = DPS368_RATE_64HZ,
        DPS368_Oversampling pressureOS = DPS368_OS_16X,
        DPS368_SamplingRate tempRate = DPS368_RATE_4HZ,
        DPS368_Oversampling tempOS = DPS368_OS_2X,
        DPS368_MeasurementMode mode = DPS368_MODE_CONT_BOTH
    );
    
    // Update-Funktion - sollte regelmäßig aufgerufen werden
    void update();
    
    // Getter-Funktionen
    float getAltitude() const { return _data.altitude_m; }
    float getPressure() const { return _data.pressure_pa; }
    float getTemperature() const { return _data.temperature_degc; }
    const DPS368_Data &getData() const { return _data; }
    bool isDataReady() const { return _dataReady; }
    
    // Debug
    void printCalibrationData() const;

private:
    // Hardware-Pins
    int _csPin, _intPin;
    SPIClass &_spi;
    
    // Kalibrierungsdaten
    DPS368_Calib _calib;
    
    // Gemessene Daten
    DPS368_Data _data;
    volatile bool _dataReady = false;
    
    // Konfiguration
    DPS368_Oversampling _pressureOS;
    DPS368_SamplingRate _pressureRate;
    
    // Konstanten
    static const float SEA_LEVEL_PRESSURE;
    
    // Private Funktionen
    bool softReset();
    bool waitForReady(uint8_t readyBits, uint32_t timeoutMs = 1000);
    bool readCalibrationCoefficients();
    void parseCalibrationBytes(const uint8_t *buffer);
    bool configureRegisters(
        DPS368_SamplingRate pressureRate,
        DPS368_Oversampling pressureOS,
        DPS368_SamplingRate tempRate,
        DPS368_Oversampling tempOS,
        DPS368_MeasurementMode mode
    );
    
    bool readMeasurement();
    void calculateAltitude();
    
    // Low-Level SPI Operationen
    void writeReg(uint8_t reg, uint8_t value);
    uint8_t readReg(uint8_t reg);
    void readRegs(uint8_t reg, uint8_t *buffer, uint8_t length);
    
    // Interrupt Handler
    static void IRAM_ATTR isrHandler(void *arg);
};

// ============================================================================
// FreeRTOS Task
// ============================================================================
void TaskBaro(void *pvParameters);

#endif // BARO_TASK_H
