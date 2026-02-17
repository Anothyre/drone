#ifndef WLED_TASK_H
#define WLED_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "data_structures.h"

extern QueueHandle_t wled_command_queue;

void TaskWLED(void *pvParameters);

#endif
