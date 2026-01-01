#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>

// GPS data structure (shared with EKF/FSM)
// TODO: Think if needed - eigentlich schoen so
typedef struct
{
    double lat, lon, alt;
    float speed, course;
    uint8_t satellites;
    float hdop;
    bool valid;
    uint32_t timestamp;
} gps_data_t;

// Global GPS data instance
extern gps_data_t gps_data; //TODO: dining philosophers??

#endif // DATA_STRUCTURES_H
