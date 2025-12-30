#include "logger_task.h"

void TaskLogger(void *pvParameters)
{
    for (;;)
    {
        // Logging task
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz
    }
}
