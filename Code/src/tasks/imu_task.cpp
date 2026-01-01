#include "tasks/imu_task.h"
#include "tasks.h"

void TaskIMU(void *pvParameters)
{
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // read IMU DMA buffer
        // run AHRS (Altitude and Heading Reference System)
        // publish attitude (double buffer)
        xTaskNotifyGive(TaskControl_Handle);
    }
}
