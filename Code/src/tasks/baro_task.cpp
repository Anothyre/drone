#include "tasks/baro_task.h"

void TaskBaro(void *pvParameters)
{
   // for (;;)// MOMENTAN NUR GEMINI CODE!
   {
#include <Arduino.h>
#include <SPI.h>
#include <cmath>

// Skalierungsfaktoren für 16x Oversampling (Standard für Drohnen)
// Siehe Datenblatt Tabelle 9: kP und kT für 16x = 253952//TODO: discuss 
const int32_t SCALING_FACTOR = 253952; 

struct BaroData {
    float pressure;    // in Pascal
    float temperature; // in Celsius
    float altitude;    // in Metern
};

class DPS368 {
private:
    int _csPin;
    // Kalibrierkoeffizienten (müssen in begin() geladen werden)
    int32_t c0, c1, c00, c10, c20, c30, c01, c11, c21;
    
    // Hilfswert für die Höhenberechnung (QNH / Luftdruck auf Meereshöhe)
    const float SEA_LEVEL_PRESSURE = 101325.0f; 

public:
    DPS368(int cs) : _csPin(cs) {}

    // Die zentrale Rechenfunktion
    BaroData update() {
        uint8_t buffer[6];
        
        // 1. Burst Read via SPI (Pressure + Temperature)
        // Adresse 0x00, MSB ist 1 für Read-Mode bei SPI
        digitalWrite(_csPin, LOW);
        SPI.transfer(0x00 | 0x80); 
        for(int i=0; i<6; i++) {
            buffer[i] = SPI.transfer(0x00);
        }
        digitalWrite(_csPin, HIGH);

        // 2. Rohwerte zusammensetzen (24-bit)
        int32_t raw_p = (int32_t)buffer[0] << 16 | (int32_t)buffer[1] << 8 | (int32_t)buffer[2];
        int32_t raw_t = (int32_t)buffer[3] << 16 | (int32_t)buffer[4] << 8 | (int32_t)buffer[5];

        // 3. 24-bit Zweierkomplement (Sign Extension)
        if (raw_p & 0x800000) raw_p -= 0x1000000;
        if (raw_t & 0x800000) raw_t -= 0x1000000;

        // 4. Skalierung (Scaling)
        float p_sc = (float)raw_p / SCALING_FACTOR;
        float t_sc = (float)raw_t / SCALING_FACTOR;

        // 5. Temperatur-Kompensation (Abschnitt 4.9.2)
        float comp_temp = (float)c0 * 0.5f + (float)c1 * t_sc;

        // 6. Druck-Kompensation (Abschnitt 4.9.1)
        // Formel: P = c00 + p_sc*(c10 + p_sc*(c20 + p_sc*c30)) + t_sc*c01 + t_sc*p_sc*(c11 + p_sc*c21)
        float comp_press = (float)c00 
                         + p_sc * ((float)c10 + p_sc * ((float)c20 + p_sc * (float)c30)) 
                         + t_sc * (float)c01 
                         + t_sc * p_sc * ((float)c11 + p_sc * (float)c21);

        // 7. Höhenberechnung (Internationale Höhenformel)
        float altitude = 44330.0f * (1.0f - powf(comp_press / SEA_LEVEL_PRESSURE, 0.1902949f));

        return {comp_press, comp_temp, altitude};
    }
};        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }

    /*  
    GEMINI ENTWURF:

    header:#include <SPI.h>
#include <Arduino.h>

class DroneBaro {
public:
    DroneBaro(int csPin, int intPin, SPIClass &spiBus);
    bool init();
    void update(); // Aufruf im Loop (z.B. 50-100Hz)
    float getAltitude(); // Gibt gefilterte Höhe zurück

private:
    int _cs, _int;
    SPIClass &_spi;
    
    // Flüchtiges Flag für Interrupt
    volatile bool _dataReady = false; 
    
    // Kalibrierdaten
    int32_t c0, c1, c00, c10, c20, c30, c01, c11, c21;
    
    // Berechnungsvariablen
    float _lastTemp = 0.0f;
    float _lastPressure = 0.0f;
    float _altitude = 0.0f;

    void readCalibration();
    void writeReg(uint8_t reg, uint8_t val);
    void readRegs(uint8_t reg, uint8_t *buf, uint8_t len);
    
    // Statische ISR Wrapper Funktion (nötig für attachInterrupt in Klassen)
    static void IRAM_ATTR isrHandler(void* arg);
};



    // WICHTIG: IRAM_ATTR sorgt beim ESP32 dafür, dass der Code im RAM liegt 
// und nicht aus dem Flash geladen werden muss (schneller, kein Crash bei Flash-Write).
void IRAM_ATTR DroneBaro::isrHandler(void* arg) {
    DroneBaro* sensor = static_cast<DroneBaro*>(arg);
    sensor->_dataReady = true;
}

bool DroneBaro::init() {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    pinMode(_int, INPUT_PULLDOWN); // Je nach Config

    _spi.begin();
    
    // 1. Reset & Koeffizienten lesen (wie im vorherigen Beispiel)
    // ... (Code zum Lesen der c0, c1... Koeffizienten hier einfügen) ...

    // 2. Interrupt aktivieren (CFG_REG 0x09)
    // Bit 4 (INT_PRS), Bit 5 (INT_TMP), Bit 2 (P_SHIFT für >8x Oversampling)
    // SPI Mode (Bit 0) nicht vergessen falls 3-Wire genutzt wird.
    writeReg(0x09, 0b00110100); // INT_PRS=1, INT_TMP=1, P_SHIFT=1

    // 3. Modus setzen: Background Mode, Pres 16x (Standard), Temp 1x
    // PRS_CFG (0x06): Rate 32Hz (101), Oversampling 16x (0100) -> 0x54
    writeReg(0x06, 0x54); 
    // TMP_CFG (0x07): Rate 1Hz, Oversampling 1x -> Default reicht oft, oder explizit setzen
    
    // MEAS_CFG (0x08): Continous Pressure & Temp (111) -> 0x07
    writeReg(0x08, 0x07);

    // 4. ESP32 Interrupt Attach
    attachInterruptArg(digitalPinToInterrupt(_int), isrHandler, this, RISING);
    
    return true;
}

void DroneBaro::update() {
    if (_dataReady) {
        _dataReady = false; // Flag resetten

        // SPI Transaktion starten (ESP32 optimiert)
        _spi.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE3)); // 10 MHz!
        digitalWrite(_cs, LOW);
        
        // Burst Read ab 0x00 (Pressure B2) bis 0x05 (Temp B0)
        // Wir senden das Registerbyte + Read-Bit (0x00 | 0x80)
        _spi.transfer(0x00 | 0x80); 
        
        uint8_t buf[6];
        // Effizienter Block-Transfer
        _spi.transferBytes(NULL, buf, 6);
        
        digitalWrite(_cs, HIGH);
        _spi.endTransaction();

        // --- HIER KOMMT DIE MATHE ---
        // Rohwerte zusammenbauen
        int32_t raw_p = (int32_t(buf[0]) << 16) | (int32_t(buf[1]) << 8) | int32_t(buf[2]);
        int32_t raw_t = (int32_t(buf[3]) << 16) | (int32_t(buf[4]) << 8) | int32_t(buf[5]);
        
        // Sign Extension (24 bit zu 32 bit signed)
        if (raw_p & 0x800000) raw_p -= 0x1000000;
        if (raw_t & 0x800000) raw_t -= 0x1000000;

        // Kompensations-Formel anwenden (siehe vorherige Antwort - zeili fragen)
        // ... calculateCompensatedPressure(raw_p, raw_t) ...
        
        // Höhenberechnung (Barometrische Höhenformel)
        // 44330 * (1.0 - pow(pressure / 101325.0, 0.1903));
        
        // Filterung (für Drohnen ESSENZIELL)
        // Einfacher IIR Filter (Complementary Filter)
        _altitude = (_altitude * 0.90f) + (new_altitude * 0.10f);
    }
}


    
    */


}