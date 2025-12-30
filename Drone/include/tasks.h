#ifndef TASKS_H
#define TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Task handles
extern TaskHandle_t TaskIMU_Handle;
extern TaskHandle_t TaskControl_Handle;
extern TaskHandle_t TaskEKF_Handle;
extern TaskHandle_t TaskGPS_Handle;
extern TaskHandle_t TaskBaro_Handle;
extern TaskHandle_t TaskADC_Handle;
extern TaskHandle_t TaskWiFi_Handle;
extern TaskHandle_t TaskFSM_Handle;
extern TaskHandle_t TaskLogger_Handle;
extern TaskHandle_t TaskHousekeeping_Handle;

// Task function declarations
void TaskIMU(void *pvParameters);
void TaskControl(void *pvParameters);
void TaskEKF(void *pvParameters);
void TaskGPS(void *pvParameters);
void TaskBaro(void *pvParameters);
void TaskADC(void *pvParameters);
void TaskWiFi(void *pvParameters);
void TaskFSM(void *pvParameters);
void TaskLogger(void *pvParameters);
void TaskHousekeeping(void *pvParameters);

// Task creation
void create_tasks(void);

#endif // TASKS_H
