#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>

// GPS data structure (shared with EKF/FSM)
// TODO: Think if needed - eigentlich schoen so
struct gps_data_t
{
    double lat, lon, alt;
    float speed, course;
    uint8_t satellites;
    float hdop;
    bool valid;
    uint32_t timestamp;
} ;


// Struktur für die Rückgabewerte
struct BaroData {
    float pressure;    // Pascal
    float temperature; // Celsius
    float altitude;    // Meter
};

// Global GPS data instance
extern gps_data_t gps_data; //TODO: dining philosophers??

#endif // DATA_STRUCTURES_H
