#ifndef SHARED_H
#define SHARED_H
#include <Arduino.h>

// Nur "extern" - das verspricht dem Compiler, dass die Variable existiert
extern QueueHandle_t fsm_command_queue;
extern QueueHandle_t fsm_event_queue;
extern QueueHandle_t sensorQueue;

void initQueues(); // Prototyp für die Initialisierung
#endif

