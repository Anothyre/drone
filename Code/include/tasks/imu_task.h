#ifndef IMU_TASK_H
#define IMU_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void TaskIMU(void *pvParameters);

#endif // IMU_TASK_H
