#include "control_task.h"
#include "tasks.h"

void TaskControl(void *pvParameters)
{
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2));
        // read attitude from ekf
        // run PID  o.AE.
        // TODO; Concept - TOGETHER
        // mix motors

        // output PWM
        vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz // TODO: adjust for all tasks
    }
}