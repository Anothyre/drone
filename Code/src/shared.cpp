#include "shared.h"
#include "tasks/fsm_task.h"
#include "tasks/wifi_task.h"
#include "tasks/wled_task.h"

// Hier tatsächlich definieren 
QueueHandle_t fsm_command_queue = NULL;
QueueHandle_t fsm_event_queue = NULL;//WHY ? 

QueueHandle_t imuQueue = NULL;
QueueHandle_t baroQueue = NULL;
QueueHandle_t gpsQueue = NULL;
QueueHandle_t  inputQueue = NULL;

QueueHandle_t ADCQueue = NULL;
QueueHandle_t wled_command_queue = NULL;

void initQueues() {
    fsm_command_queue = xQueueCreate(FSM_COMMAND_QUEUE_LEN, sizeof(control_packet_t));
    fsm_event_queue = xQueueCreate(FSM_EVENT_QUEUE_LEN, sizeof(event_t));
    imuQueue = xQueueCreate(1, sizeof(float));
    ADCQueue = xQueueCreate(ADC_QUEUE_LENGHT, sizeof(ADCSample_t));
    inputQueue = xQueueCreate(5, sizeof(control_packet_t));//TODO: consider lenghts
    wled_command_queue = xQueueCreate(5, sizeof(control_packet_t));
}
