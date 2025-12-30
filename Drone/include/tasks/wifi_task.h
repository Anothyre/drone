#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>

#define CONTROL_PORT 14550
#define TELEMETRY_PORT 14551
#define CONTROL_TIMEOUT_MS 200 // tweek

typedef struct __attribute__((packed)) // no padding
{
    uint32_t magic; // 0x44524F4E ("DRON")
    uint16_t seq;   // sequence number
    uint32_t timestamp_ms;
    float x; // velocity / position, absolute or relative?
    float y;
    float z;
    uint8_t mode; // one for each flight mode, *_cmd state and none.
    uint16_t crc; // CRC-16-IBM checksum
} control_packet_t;

void TaskWiFi(void *pvParameters);

#endif // WIFI_TASK_H
