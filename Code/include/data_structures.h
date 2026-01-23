#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>

// GPS data structure (shared with EKF/FSM)
// TODO: Think if needed - eigentlich schoen so



// Struktur für die Rückgabewerte
struct BaroData {
    float pressure;    // Pascal
    float temperature; // Celsius
    float altitude;    // Meter
};


typedef struct {
    double lat, lon, alt;
    float speed, course;
    uint8_t satellites;
    float hdop;
    bool valid;
    uint32_t timestamp;
} gps_data_t;

extern gps_data_t gps_data;


#endif // DATA_STRUCTURES_H
