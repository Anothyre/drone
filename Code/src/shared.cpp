#include "shared.h"

// Hier tatsächlich definieren (ohne extern!)
QueueHandle_t fsm_command_queue = NULL;
QueueHandle_t fsm_event_queue = NULL;//WHY ? Safety! Pointers are not NULL initialized but point to a "random" memmory
QueueHandle_t sensorQueue = NULL;

void initQueues() {
    fsm_command_queue = xQueueCreate(10, sizeof(uint8_t));
    fsm_event_queue = xQueueCreate(10, sizeof(int));
    sensorQueue = xQueueCreate(1, sizeof(float));
    ADCQueue = xQueueCreate(ADC_QUEUE_LENGHT, sizeof(ADCSample_t));
}