#include "tasks/ekf_task.h"
#include "tasks/gps_task.h"
#include "data_structures.h"
#include <TinyGPS++.h>


// Global GPS data instance
extern TinyGPSPlus gps;
extern gps_data_t gps_data; //TODO: KAS

void TaskEKF(void *pvParameters)
{
    bool useGps; 
    for (;;)
    
    {
        useGps = gps_data.valid;

        
        
            
        // Extended Kalman Filter implementation OR DIFFERENT SENSOR FUSION APPROCHE
        // TODO: TOGETHER
        //USe https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/04-Stream-and-message-buffers/01-RTOS-stream-and-message-buffers
    // to communicate between cores 
         // probaly to complex and unneccesarry for our purpose

        // care if gps is valid

        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}
