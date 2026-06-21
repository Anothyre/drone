#ifndef SHARED_H
#define SHARED_H
#include <Arduino.h>

// Nur "extern" - das verspricht dem Compiler, dass die Variable existiert
extern QueueHandle_t fsm_command_queue;
extern QueueHandle_t fsm_event_queue;
extern QueueHandle_t imuQueue;
extern QueueHandle_t baroQueue;
extern QueueHandle_t gpsQueue;
extern QueueHandle_t ekfQueue;
extern QueueHandle_t inputQueue;
extern QueueHandle_t ADCQueue;
extern QueueHandle_t wled_command_queue;

#define FSM_COMMAND_QUEUE_LEN 10
#define FSM_EVENT_QUEUE_LEN 10
#define ADC_QUEUE_LENGHT 32

typedef struct{
    float batteryCurrent;
    float batteryVoltage;
} ADCSample_t;

void initQueues(); // Prototyp für die Initialisierung
#endif

