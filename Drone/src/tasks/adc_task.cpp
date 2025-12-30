#include "adc_task.h"
// analog digital converter task

void TaskADC(void *pvParameters)
{
    for (;;)
    {
        // TODO: ADC reading for current/voltage monitoring
        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz
    }
}
