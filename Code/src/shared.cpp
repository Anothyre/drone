#include "shared.h"

// Hier tatsächlich definieren (ohne extern!)
QueueHandle_t fsm_command_queue = NULL;
QueueHandle_t fsm_event_queue = NULL;//WHY ?
QueueHandle_t sensorQueue = NULL;

void initQueues() {
    fsm_command_queue = xQueueCreate(10, sizeof(control_packet_t));
    fsm_event_queue = xQueueCreate(10, sizeof(int));
    sensorQueue = xQueueCreate(1, sizeof(float));
}