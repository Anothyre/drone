#include "tasks/fsm_task.h"
#include "tasks/wifi_task.h"
#include <Arduino.h>

void TaskFSM(void *pvParameters)
{
    control_packet_t cmd;

    for (;;)

    {
        //TODO:
    // EV_NONE,
    // EV_INIT_COMPLETE,
    // EV_TESTS_PASS,
    // EV_ERROR,
    // EV_TAKEOFF_CMD,
    // EV_TOUCHDOWN,
    // EV_TARGET_ALT_REACHED,
    // EV_STABILIZE_CMD,
    // EV_ALT_HOLD_CMD,
    // EV_POS_HOLD_CMD,
    // EV_MISSION_CMD,
    // EV_ABORT_CMD,
    // EV_MISSION_COMPLETE,
    // EV_LAND_CMD,
    // EV_EXCEPTION
        if (xQueueReceive(fsm_command_queue, &cmd, pdMS_TO_TICKS(20)))
        {
            if (cmd.mode == EV_ERROR) //error
            {
                //TODO: Failsafe
            }
            if (cmd.mode == EV_ARM_CMD) // arm command
            {
                //TODO: check arming conditions
            }

            // update setpoints

            // TODO: implement FSM logic from prototyp #Steuerzentrale
        }
    }
}
