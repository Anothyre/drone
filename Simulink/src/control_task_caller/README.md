# Control Task Simulink Adapter

This folder contains a C-compatible wrapper of the control logic from `Code/src/tasks/control_task.cpp` for use with a Simulink C Caller block.

## Files

- `control_task_simulink.h`
- `control_task_simulink.cpp`

## Exported API

- `drone_control_default_params(DroneControlParams *params)`
- `drone_control_init(const DroneControlParams *params)`
- `drone_control_reset(void)`
- `drone_control_step(...)`

`drone_control_step` has scalar inputs and pointer-based outputs (`motor_fl`, `motor_fr`, `motor_rl`, `motor_rr`) so Simulink can generate block ports directly.

## Recommended C Caller setup

1. Model Configuration Parameters -> Simulation Target:
   - Language: `C++`
   - Include directories: add path to this folder
   - Source files: `control_task_simulink.cpp`
2. In the C Caller block:
   - Header: `control_task_simulink.h`
   - Call once at startup: `drone_control_init`
   - Call every sample step: `drone_control_step`
   - Optional reset event: `drone_control_reset`

## Notes

- No `while` loop and no RTOS dependency in this module.
- Controller states are persistent between calls via static module state.
- Output signals are normalized and clamped to `[0, 1]`.
