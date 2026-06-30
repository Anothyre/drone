#include "tasks/logger_task.h"

void TaskLogger(void *pvParameters)
{
    for (;;)
    {

        //TODO: Implement reliable SD-card logging for telemetry and flight data.
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz
    }
}
