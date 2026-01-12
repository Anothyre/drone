#include "tasks/logger_task.h"

void TaskLogger(void *pvParameters)
{
    for (;;)
    {

        // Logging task
        //TODO: writre on SD
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz
    }
}
