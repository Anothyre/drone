#include "tasks/fsm_task.h"
#include "tasks/wifi_task.h"
#include <Arduino.h>

void TaskFSM(void *pvParameters)
{
    control_packet_t cmd;

    for (;;)
    {
        if (xQueueReceive(fsm_command_queue, &cmd, pdMS_TO_TICKS(20)))
        {
            if (cmd.mode ==) // arm command
            {
                // check arming conditions
            }
            // update setpoints

            // TODO: implement FSM logic from prototyp #Steuerzentrale
        }
    }
}
