#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>

// Network control packet structure
// WICHTIG: __attribute__((packed)) verhindert, dass C++ leere Bytes (Padding) einfügt.
// Das muss exakt dem Python struct.pack("<I H I f f f f B H", ...) entsprechen.
// Gesamtgröße: exakt 31 Bytes.
typedef struct __attribute__((packed)) {
    uint32_t magic;         // I (4 Byte) - 0x44524F4E ("DRON")
    uint16_t seq;           // H (2 Byte)
    uint32_t timestamp_ms;  // I (4 Byte)
    float x;                // f (4 Byte)
    float y;                // f (4 Byte)
    float z;                // f (4 Byte)
    float yaw;              // f (4 Byte)
    uint8_t mode;           // B (1 Byte)
    uint16_t crc;           // H (2 Byte)
} control_packet_t;

// GPS data structure (shared with EKF/FSM)
// TODO: Think if needed - eigentlich schoen so



// Struktur für die Rückgabewerte
struct BaroData {
    float pressure;    // Pascal
    float temperature; // Celsius
    float altitude;    // Meter
};
struct IMUData {
    float accelX; // m/s²
    float accelY; // m/s²
    float accelZ; // m/s²
    float gyroX;  // °/s
    float gyroY;  // °/s
    float gyroZ;  // °/s
    //TODO: actuall values
    };


 struct GPSData {
    double lat, lon, alt;
    float speed, course;
    uint8_t satellites;
    float hdop;
    bool valid;
    uint32_t timestamp;
};

typedef struct {
    float pitch;
    float roll;
    float yaw;
    float altitude_m;
    float posX;
    float posY;
    float posZ;
    uint32_t timestamp_ms;
    bool valid_attitude;
    bool valid_altitude;
    bool valid_position;
} EKFState_t;

extern GPSData gps_data;

#endif // DATA_STRUCTURES_H
