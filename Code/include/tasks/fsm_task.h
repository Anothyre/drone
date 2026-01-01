#ifndef FSM_TASK_H
#define FSM_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

typedef enum
{
    ST_BOOT,
    ST_SELF_TEST,
    ST_IDLE,
    ST_ARMED,
    ST_FLIGHT,
    ST_FAILSAFE
} state_t;

typedef enum
{
    FL_TAKEOFF,
    FL_STABILIZE,
    FL_ALT_HOLD,
    FL_POS_HOLD,
    FL_MISSION,
    FL_LANDING
} flight_t;

typedef enum
{
    EV_NONE,
    EV_INIT_COMPLETE,
    EV_TESTS_PASS,
    EV_ERROR,
    EV_ARM_CMD,
    EV_TAKEOFF_CMD,
    EV_TOUCHDOWN,
    EV_TARGET_ALT_REACHED,
    EV_STABILIZE_CMD,
    EV_ALT_HOLD_CMD,
    EV_POS_HOLD_CMD,
    EV_MISSION_CMD,
    EV_ABORT_CMD,
    EV_MISSION_COMPLETE,
    EV_LAND_CMD,
    EV_EXCEPTION
} event_t;

void TaskFSM(void *pvParameters);
extern QueueHandle_t fsm_command_queue;
extern QueueHandle_t fsm_event_queue;

#endif // FSM_TASK_H
