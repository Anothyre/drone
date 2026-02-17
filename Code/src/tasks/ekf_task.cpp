#include "tasks/ekf_task.h"
#include "tasks/gps_task.h"
#include "data_structures.h"
#include <TinyGPS++.h>
#include "shared.h"


// Global GPS data instance
extern TinyGPSPlus gps;


void TaskEKF(void *pvParameters)
{
    bool useGps; 
    for (;;)
    
    {


    bool has_new = false;

    


    IMUData  IMUpkt,  IMUlatest;
    has_new = false;

    while (xQueueReceive(imuQueue, &IMUpkt, 0) == pdPASS) {
    IMUlatest = IMUpkt;      
    has_new = true;
    }

    BaroData baroPkt, baroLatest;
    has_new = false;

    while (xQueueReceive(baroQueue, &baroPkt, 0) == pdPASS) {
    baroLatest = baroPkt;     
    has_new = true;
    }


    GPSData gpsPkt, gpsLatest;
    has_new = false;
    while (xQueueReceive(gpsQueue, &gpsPkt, 0) == pdPASS) {
    gpsLatest = gpsPkt;      
    has_new = true;
    }


        
        
            
        // Extended Kalman Filter implementation OR DIFFERENT SENSOR FUSION APPROCHE
        // TODO: TOGETHER
        //USe https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/04-Stream-and-message-buffers/01-RTOS-stream-and-message-buffers
    // to communicate between cores 
         // probaly to complex and unneccesarry for our purpose

        // care if gps is valid

        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}
