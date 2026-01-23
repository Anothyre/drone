#ifndef HOUSEKEEPING_TASK_H
#define HOUSEKEEPING_TASK_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>

void TaskHousekeeping(void *pvParameters);

#endif // HOUSEKEEPING_TASK_H
