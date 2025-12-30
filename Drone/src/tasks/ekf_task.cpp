#include "ekf_task.h"

void TaskEKF(void *pvParameters)
{
    for (;;)
    {
        // Extended Kalman Filter implementation
        // TOGETHER

        // care if gps is valid

        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}
