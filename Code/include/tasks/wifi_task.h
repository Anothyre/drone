#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include "data_structures.h"

#define CONTROL_PORT 14550
#define TELEMETRY_PORT 14551
#define CONTROL_TIMEOUT_MS 200 // tweek

void TaskWiFi(void *pvParameters);

#endif // WIFI_TASK_H
