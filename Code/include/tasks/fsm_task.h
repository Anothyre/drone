#ifndef FSM_TASK_H
#define FSM_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "shared.h"

// ============ FSM Configuration Constants ============
#define FSM_TARGET_TAKEOFF_ALTITUDE 2.0f        // meters
#define FSM_ALT_THRESHOLD 0.2f                  // meters (±0.2m for target_alt_reached)
#define FSM_TAKEOFF_TIMEOUT_MS 30000            // 30 seconds max for takeoff
#define FSM_LANDING_DESCENT_RATE 0.5f           // m/s
#define FSM_AUTO_DISARM_TIMEOUT_MS 3000         // 3 seconds after touchdown
#define FSM_BATTERY_CRITICAL_PERCENT 10.0f      // Emergency landing trigger
#define FSM_BATTERY_LOW_PERCENT 20.0f           // Warning threshold
#define FSM_TIMEOUT_THRESHOLD_MS 5000           // Generic timeout for state transitions
typedef enum
{
    ST_BOOT,
    ST_SELF_TEST,
    ST_IDLE,
    ST_ARMED,
    ST_TAKEOFF,
    ST_STABILIZE,
    ST_ALT_HOLD,
    ST_POS_HOLD,
    ST_MISSION,
    ST_LANDING,
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
    EV_ARM_CMD,//CMD
    EV_TAKEOFF_CMD,
    EV_TOUCHDOWN,
    EV_TARGET_ALT_REACHED,
    EV_STABILIZE_CMD,//CMD
    EV_ALT_HOLD_CMD,//CMD
    EV_POS_HOLD_CMD,//CMD
    EV_MISSION_CMD,//CMD
    EV_ABORT_CMD,//CMD
    EV_MISSION_COMPLETE,
    EV_LAND_CMD,//CMD
    EV_EXCEPTION
} event_t;

void TaskFSM(void *pvParameters);
extern QueueHandle_t fsm_command_queue;
extern QueueHandle_t fsm_event_queue;

// FSM State and context variables
extern state_t fsm_current_state;
extern flight_t fsm_current_flight_mode;

// Position/Velocity tracking
typedef struct {
    float x, y, z;
} vector_t;

extern vector_t fsm_position;
extern vector_t fsm_velocity;
extern vector_t fsm_target_position;
extern vector_t fsm_home_position;

// System status
extern float fsm_battery_level;         // 0-100%
extern uint32_t fsm_last_state_change_ms;
extern uint32_t fsm_state_entry_time_ms;
extern bool fsm_tests_passed;
extern bool fsm_armed;

#endif // FSM_TASK_H
