#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void TaskLogger(void *pvParameters);

#endif // LOGGER_TASK_H
