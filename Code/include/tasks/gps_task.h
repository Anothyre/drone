#ifndef GPS_TASK_H
#define GPS_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "data_structures.h"

// GPS UART Configuration
#define GPS_RX_PIN 16 // ESP32S3 RX
#define GPS_TX_PIN 17 // ESP32S3 TX
#define GPS_UART 1    // Hardware UART 1
#define GPS_BAUD 9600

void TaskGPS(void *pvParameters);

#endif // GPS_TASK_H
