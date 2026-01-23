#include "FreeRTOS.h"
extern QueueHandle_t sensorQueue;

sensorQueue = xQueueCreate(1, sizeof(float));


