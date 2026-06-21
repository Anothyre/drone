#include "tasks.h"
#include "core_config.h"
#include "tasks/imu_task.h"
#include "tasks/control_task.h"
#include "tasks/ekf_task.h"
#include "tasks/gps_task.h"
#include "tasks/baro_task.h"
#include "tasks/adc_task.h"
#include "tasks/wifi_task.h"
#include "tasks/fsm_task.h"
#include "tasks/logger_task.h"
#include "tasks/housekeeping_task.h"
#include "tasks/wled_task.h"


// TODO:Remove Magic Numbers (prioritysy,cores,stacksizes)

// Task handle definitions
TaskHandle_t TaskIMU_Handle = NULL;
TaskHandle_t TaskControl_Handle = NULL;
TaskHandle_t TaskEKF_Handle = NULL;
TaskHandle_t TaskGPS_Handle = NULL;
TaskHandle_t TaskBaro_Handle = NULL;
TaskHandle_t TaskADC_Handle = NULL;
TaskHandle_t TaskWiFi_Handle = NULL;
TaskHandle_t TaskFSM_Handle = NULL;
TaskHandle_t TaskLogger_Handle = NULL;
TaskHandle_t TaskHousekeeping_Handle = NULL;
TaskHandle_t TaskWLED_Handle = NULL;

 void create_tasks(void)
{
    // =======================
    // REAL-TIME TASKS (CORE 1)
    // =======================

    xTaskCreatePinnedToCore(
        TaskIMU,
        "IMU_Task",
        8192,
        NULL,
        6, // highest priority
        &TaskIMU_Handle,
        CORE_REALTIME);

    xTaskCreatePinnedToCore(
        TaskControl, // PID
        "Control_Task",
        8192,
        NULL,
        5,
        &TaskControl_Handle,
        CORE_REALTIME);

    // =======================
    // ESTIMATION
    // =======================

    xTaskCreatePinnedToCore(
        TaskEKF, // Extended Kalman Filter
        "EKF_Task",
        8192,
        NULL,
        4,
        &TaskEKF_Handle,
        CORE_COMMS);

    // =======================
    // SENSOR TASKS
    // =======================

    xTaskCreatePinnedToCore(
        TaskGPS, // gps task
        "GPS_Task",
        4096,
        NULL,
        3,
        &TaskGPS_Handle,
        CORE_COMMS);

    xTaskCreatePinnedToCore(
        TaskBaro, // barometer task
        "Baro_Task",
        4096,
        NULL,
        3,
        &TaskBaro_Handle,
        CORE_COMMS);

    xTaskCreatePinnedToCore(
        TaskADC, // adc task for current/voltage monitoring
        "ADC_Task",
        4096,
        NULL,
        3,
        &TaskADC_Handle,
        CORE_COMMS);

    // =======================
    // FSM / SUPERVISOR
    // =======================

    xTaskCreatePinnedToCore(
        TaskFSM, // finite state machine
        "FSM_Task",
        4096,
        NULL,
        4, // higher than comms
        &TaskFSM_Handle,
        CORE_COMMS);

    // =======================
    // COMMUNICATION
    // =======================

    xTaskCreatePinnedToCore(
        TaskWiFi, // communication task
        "WiFi_Task",
        8192,
        NULL,
        2,
        &TaskWiFi_Handle,
        CORE_COMMS);

    // // =======================
    // // LOGGING (LOW PRIORITY)
    // // =======================

    // xTaskCreatePinnedToCore(
    //     TaskLogger, // blackbox logging task & telemetrie
    //     "Logger_Task",
    //     4096,
    //     NULL,
    //     1,
    //     &TaskLogger_Handle,
    //     CORE_COMMS);

    // // =======================
    // // HOUSEKEEPING / OTA / CLI
    // // =======================

    // xTaskCreatePinnedToCore(
    //     TaskHousekeeping,
    //     "Housekeeping_Task",
    //     4096,
    //     NULL,
    //     1,
    //     &TaskHousekeeping_Handle,
    //     CORE_COMMS);

    // =======================
    // VISUALIZATION (WLED)
    // =======================

    xTaskCreatePinnedToCore(
        TaskWLED,
        "WLED_Task",
        4096,
        NULL,
        1,
        &TaskWLED_Handle,
        CORE_COMMS);
}
