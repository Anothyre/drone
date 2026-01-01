#include "tasks/baro_task.h"

void TaskBaro(void *pvParameters)
{
    for (;;)
    {
        // TODO: Barometer sensor reading
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}
