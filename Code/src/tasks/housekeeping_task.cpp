#include "tasks/housekeeping_task.h"

void TaskHousekeeping(void *pvParameters)
{
    for (;;)
    {
        // Housekeeping: OTA, CLI, health checks
        ´ vTaskDelay(pdMS_TO_TICKS(1000)); // 1Hz
    }
}
