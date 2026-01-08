#include "tasks/ekf_task.h"
#include "tasks/gps_task.h"
#include <TinyGPS++.h>

void TaskEKF(void *pvParameters)
{
    bool useGps; 
    for (;;)
    {
        useGps = gps_data.valid;
        
            
        // Extended Kalman Filter implementation
        // TODO: TOGETHER

         // probaly to complex and unneccesarry for our purpose

        // care if gps is valid

        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}
