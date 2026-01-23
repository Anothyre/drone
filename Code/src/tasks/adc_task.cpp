#include "tasks/adc_task.h"
// analog digital converter task for ???

void TaskADC(void *pvParameters)
{
    for (;;)
    {
        //BRANDNER
        // TODO: ADC reading for current/voltage monitoring
        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz
    }
}
