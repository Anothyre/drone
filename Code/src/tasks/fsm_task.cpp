#include "tasks/fsm_task.h"
#include "tasks/wifi_task.h"
#include "tasks/control_task.h"
#include "tasks/logger_task.h"
#include "hardware.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "data_structures.h"
#include <math.h>

// ============ FSM State and Context Variables ============
state_t fsm_current_state = ST_BOOT;
flight_t fsm_current_flight_mode = FL_TAKEOFF;

// Position/Velocity tracking
vector_t fsm_position = {0.0f, 0.0f, 0.0f};
vector_t fsm_velocity = {0.0f, 0.0f, 0.0f};
vector_t fsm_target_position = {0.0f, 0.0f, 0.0f};
vector_t fsm_home_position = {0.0f, 0.0f, 0.0f};

// System status
float fsm_battery_level = 100.0f;           // 0-100%
uint32_t fsm_last_state_change_ms = 0;
uint32_t fsm_state_entry_time_ms = 0;
bool fsm_tests_passed = false;
bool fsm_armed = false;

// ============ Forward Declarations (dummy implementations) ============

// Initialize all peripherals (hardware init)
void fsm_initialize_peripherals(void)
{
    Serial.println("[FSM] Initializing peripherals...");
    hardware_init();
    hardware_set_system_status(true, false, false, false, false, false, false);
    hardware_beep(2, 80, 60);
}

// Run self-test on all systems
bool fsm_run_self_tests(void)
{
    //TODO: Implement self-test logic:
    //  - Check IMU sensor health
    //  - Check Barometer sensor health
    //  - Check GPS sensor health
    //  - Check Motor connectivity
    //  - Check Battery voltage
    //  - Return true if all pass, false if any fail
    Serial.println("[FSM] Running self-tests...");
    return true; // Dummy: assume tests pass
}

// Arm motors with safety checks
void fsm_arm_motors(void)
{
    Serial.println("[FSM] Arming motors...");
    hardware_set_armed(true);
    hardware_set_motors_enabled(true);
    hardware_beep(3, 80, 60);
    fsm_armed = true;
}

// Disarm motors safely
void fsm_disarm_motors(void)
{
    Serial.println("[FSM] Disarming motors...");
    hardware_set_armed(false);
    hardware_set_motors_enabled(false);
    hardware_beep(1, 60, 40);
    fsm_armed = false;
}

// Initiate takeoff sequence
void fsm_begin_takeoff(void)
{
    // TODO: Implement a real takeoff sequence with altitude target and throttle ramp.
    //  - Save current position as home_position
    //  - Enable Z-axis position controller
    //  - Gradually increase thrust
    //  - Set target altitude to FSM_TARGET_TAKEOFF_ALTITUDE
    //  - Start takeoff timeout timer
    Serial.println("[FSM] Beginning takeoff...");
    fsm_home_position = fsm_position;
    fsm_target_position.z = fsm_home_position.z + FSM_TARGET_TAKEOFF_ALTITUDE;
}

// Check if target altitude reached
bool fsm_check_altitude_reached(void)
{
    float alt_error = fabsf(fsm_position.z - fsm_target_position.z);
    return (alt_error < FSM_ALT_THRESHOLD);
}

// Switch to stabilize flight mode
void fsm_switch_to_stabilize(void)
{
    // TODO: Switch the controller into a real stabilize mode and feed attitude setpoints.
    //  - Activate velocity controllers for X, Y, Z
    //  - Enable attitude stabilization (roll, pitch, yaw)
    //  - Configure TaskControl for stabilize mode
    Serial.println("[FSM] Switching to STABILIZE mode...");
    fsm_current_flight_mode = FL_STABILIZE;
}

// Switch to altitude hold mode
void fsm_switch_to_alt_hold(void)
{
    //TODO: Enable altitude hold control:
    //  - Keep Z-position constant
    //  - Regulate X,Y velocity (drift compensation)
    //  - Configure TaskControl for alt_hold mode
    Serial.println("[FSM] Switching to ALT_HOLD mode...");
    fsm_current_flight_mode = FL_ALT_HOLD;
}

// Switch to position hold mode
void fsm_switch_to_pos_hold(void)
{
    //TODO: Enable position hold control:
    //  - Hold X, Y, Z position
    //  - Active wind/drift compensation
    //  - Configure TaskControl for pos_hold mode
    Serial.println("[FSM] Switching to POS_HOLD mode...");
    fsm_current_flight_mode = FL_POS_HOLD;
}

// Initiate landing sequence
void fsm_begin_landing(void)
{
    // TODO: Implement a controlled descent and touchdown detection for landing.
    //  - Hold X, Y position at current location
    //  - Gradually reduce Z thrust
    //  - Set descent rate to FSM_LANDING_DESCENT_RATE m/s
    //  - Monitor for touchdown detection
    Serial.println("[FSM] Beginning landing...");
    fsm_current_flight_mode = FL_LANDING;
}

// Check if drone has landed
bool fsm_check_touchdown_detected(void)
{
    //TODO: Implement touchdown detection:
    //  - Check barometer (altitude near ground level)
    //  - Check accelerometer (Z-axis acceleration < threshold)
    //  - Check vertical velocity (approaching zero)
    //  - Verify all conditions stable for ~1 second
    //  - Return true when all conditions met
    return false; // Dummy: no touchdown yet
}

// Handle failsafe procedure
void fsm_enter_failsafe(void)
{
    Serial.println("[FSM] ENTERING FAILSAFE MODE!");
    hardware_set_warning(true);
    hardware_set_motors_enabled(false);
    hardware_beep(5, 120, 80);
}

// Update battery status from ADC
void fsm_update_battery_level(void)
{
    fsm_battery_level = max(0.0f, fsm_battery_level - 0.01f);
    hardware_set_battery_low(fsm_battery_level <= FSM_BATTERY_LOW_PERCENT);
}

// Update position from EKF
void fsm_update_position(void)
{
    // TODO: Read the fused EKF state and update position/velocity for navigation.
    //  - Get position (x, y, z) from EKF output
    //  - Update fsm_position
    //  - Update fsm_velocity
}

// Dummy mission load
void fsm_load_mission(void)
{
    //TODO: Implement mission loading:
    //  - Read waypoint data from storage/ground station
    //  - Populate mission waypoint queue
    //  - Configure navigation parameters
    Serial.println("[FSM] Loading mission...");
}

// Check mission completion
bool fsm_check_mission_complete(void)
{
    //TODO: Implement mission completion check:
    //  - Check if all waypoints visited
    //  - Verify final position reached
    //  - Return true when mission complete
    return false; // Dummy
}

// ============ FSM State Transition Handler ============

void fsm_handle_state_transition(state_t new_state)
{
    if (new_state == fsm_current_state) {
        return; // No state change
    }

    fsm_current_state = new_state;
    fsm_last_state_change_ms = millis();
    fsm_state_entry_time_ms = millis();

    Serial.print("[FSM] State transition: ");
    Serial.println((int)new_state);
}

// ============ FSM Event Processor ============

void fsm_process_command(control_packet_t cmd)
{
    event_t event = (event_t)cmd.mode;

    switch (fsm_current_state) {

    // ========== STATE: BOOT ==========
    case ST_BOOT:
        if (event == EV_NONE || event == EV_INIT_COMPLETE) {
            fsm_initialize_peripherals();
            fsm_handle_state_transition(ST_SELF_TEST);
        }
        break;

    // ========== STATE: SELF_TEST ==========
    case ST_SELF_TEST:
        if (event == EV_TESTS_PASS) {
            fsm_tests_passed = fsm_run_self_tests();
            if (fsm_tests_passed) {
                Serial.println("[FSM] All self-tests PASSED");
                fsm_handle_state_transition(ST_IDLE);
            } else {
                Serial.println("[FSM] Self-tests FAILED");
                fsm_handle_state_transition(ST_FAILSAFE);
            }
        }
        if (event == EV_ERROR) {
            fsm_handle_state_transition(ST_FAILSAFE);
        }
        break;

    // ========== STATE: IDLE ==========
    case ST_IDLE:
        if (event == EV_ARM_CMD) {
            //TODO: Verify arming conditions:
            //  - All tests passed
            //  - Battery above critical threshold
            //  - No active errors/exceptions
            bool can_arm = fsm_tests_passed && (fsm_battery_level > FSM_BATTERY_CRITICAL_PERCENT);
            if (can_arm) {
                fsm_arm_motors();
                fsm_handle_state_transition(ST_ARMED);
            } else {
                Serial.println("[FSM] Arming conditions not met!");
            }
        }
        if (event == EV_ERROR) {
            fsm_handle_state_transition(ST_FAILSAFE);
        }
        break;

    // ========== STATE: ARMED ==========
    case ST_ARMED:
        if (event == EV_TAKEOFF_CMD) {
            fsm_begin_takeoff();
            fsm_handle_state_transition(ST_TAKEOFF);
        }
        if (event == EV_ARM_CMD || event == EV_ERROR) {
            // Timeout or explicit disarm
            fsm_disarm_motors();
            fsm_handle_state_transition(ST_IDLE);
        }
        if (event == EV_ERROR) {
            fsm_handle_state_transition(ST_FAILSAFE);
        }
        break;

    // ========== STATE: TAKEOFF ==========
    case ST_TAKEOFF:
        if (fsm_check_altitude_reached()) {
            fsm_switch_to_alt_hold();
            fsm_handle_state_transition(ST_ALT_HOLD);
        }
        if (event == EV_TOUCHDOWN) {
            // Takeoff failed or aborted
            fsm_disarm_motors();
            fsm_handle_state_transition(ST_IDLE);
        }
        //TODO: Implement takeoff timeout check:
        //  - If time in ST_TAKEOFF > FSM_TAKEOFF_TIMEOUT_MS
        //  - Emergency landing to failsafe
        if (event == EV_ERROR || event == EV_EXCEPTION) {
            fsm_handle_state_transition(ST_FAILSAFE);
        }
        break;

    // ========== STATE: STABILIZE ==========
    case ST_STABILIZE:
        if (event == EV_ALT_HOLD_CMD) {
            fsm_switch_to_alt_hold();
            fsm_handle_state_transition(ST_ALT_HOLD);
        }
        if (event == EV_POS_HOLD_CMD) {
            fsm_switch_to_pos_hold();
            fsm_handle_state_transition(ST_POS_HOLD);
        }
        if (event == EV_LAND_CMD) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_ERROR || event == EV_EXCEPTION) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        break;

    // ========== STATE: ALT_HOLD ==========
    case ST_ALT_HOLD:
        if (event == EV_STABILIZE_CMD) {
            fsm_switch_to_stabilize();
            fsm_handle_state_transition(ST_STABILIZE);
        }
        if (event == EV_POS_HOLD_CMD) {
            fsm_switch_to_pos_hold();
            fsm_handle_state_transition(ST_POS_HOLD);
        }
        if (event == EV_LAND_CMD) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_ERROR || event == EV_EXCEPTION) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        break;

    // ========== STATE: POS_HOLD ==========
    case ST_POS_HOLD:
        if (event == EV_STABILIZE_CMD) {
            fsm_switch_to_stabilize();
            fsm_handle_state_transition(ST_STABILIZE);
        }
        if (event == EV_ALT_HOLD_CMD) {
            fsm_switch_to_alt_hold();
            fsm_handle_state_transition(ST_ALT_HOLD);
        }
        if (event == EV_MISSION_CMD) {
            fsm_load_mission();
            fsm_handle_state_transition(ST_MISSION);
        }
        if (event == EV_LAND_CMD) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_ABORT_CMD) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_ERROR || event == EV_EXCEPTION) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        break;

    // ========== STATE: MISSION ==========
    case ST_MISSION:
        if (fsm_check_mission_complete()) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_MISSION_COMPLETE) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_ABORT_CMD) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        if (event == EV_ERROR || event == EV_EXCEPTION) {
            fsm_begin_landing();
            fsm_handle_state_transition(ST_LANDING);
        }
        //TODO: Implement mission navigation:
        //  - Update target waypoint from mission queue
        //  - Monitor drone progress toward waypoint
        //  - Advance to next waypoint when reached
        break;

    // ========== STATE: LANDING ==========
    case ST_LANDING:
        if (fsm_check_touchdown_detected()) {
            fsm_disarm_motors();
            //TODO: Implement post-landing delay:
            //  - Wait FSM_AUTO_DISARM_TIMEOUT_MS after touchdown
            //  - Ensure drone fully settled
            //  - Then transition to IDLE
            fsm_handle_state_transition(ST_IDLE);
        }
        if (event == EV_ERROR || event == EV_EXCEPTION) {
            // Stay in landing, attempt controlled descent
            fsm_begin_landing();
        }
        break;

    // ========== STATE: FAILSAFE ==========
    case ST_FAILSAFE:
        fsm_enter_failsafe();
        //TODO: Implement failsafe recovery:
        //  - Log error cause
        //  - Send telemetry notification
        //  - Wait for system stabilization
        //  - Allow transition back to IDLE only after manual reset
        if (event == EV_INIT_COMPLETE) {
            // Manual reset from failsafe
            fsm_disarm_motors();
            fsm_handle_state_transition(ST_IDLE);
        }
        break;

    default:
        Serial.println("[FSM] Unknown state!");
        break;
    }
}

// ============ FreeRTOS Task Main Loop ============

void TaskFSM(void *pvParameters)
{
    control_packet_t cmd;
    event_t evt;
    const TickType_t tick_rate = pdMS_TO_TICKS(50); // 50ms loop rate

    // Initial state: BOOT
    fsm_handle_state_transition(ST_BOOT);

    for (;;)
    {
        // Update sensor data and system status
        fsm_update_battery_level();
        fsm_update_position();

        // Check for critical battery level
        if (fsm_battery_level < FSM_BATTERY_CRITICAL_PERCENT && 
            (fsm_current_state == ST_ARMED || fsm_current_state == ST_TAKEOFF || 
             fsm_current_state == ST_STABILIZE || fsm_current_state == ST_ALT_HOLD ||
             fsm_current_state == ST_POS_HOLD || fsm_current_state == ST_MISSION)) {
            Serial.println("[FSM] CRITICAL BATTERY - EMERGENCY LANDING");
            cmd.mode = EV_EXCEPTION;
            fsm_process_command(cmd);
        }

        // Process incoming events first (higher priority than regular commands)
        if (xQueueReceive(fsm_event_queue, &evt, 0))
        {
            control_packet_t evt_cmd = {};
            evt_cmd.mode = (uint8_t)evt;
            fsm_process_command(evt_cmd);
        }

        // Process incoming commands from queue
        if (xQueueReceive(fsm_command_queue, &cmd, 0))
        {
            fsm_process_command(cmd);
        }

        // State-specific periodic checks
        //TODO: Add periodic timeout checks for states:
        //  - TAKEOFF: Check for timeout > FSM_TAKEOFF_TIMEOUT_MS
        //  - MISSION: Monitor navigation progress
        //  - LANDING: Monitor descent rate and altitude
        //  - FAILSAFE: Check error condition status

        vTaskDelay(tick_rate);
    }
}
