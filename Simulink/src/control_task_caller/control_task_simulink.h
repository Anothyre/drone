#ifndef DRONE_CONTROL_SIMULINK_H
#define DRONE_CONTROL_SIMULINK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DroneControlParams {
    float attitude_kp_pitch;
    float attitude_kp_roll;
    float attitude_kp_yaw;

    float rate_kp_pitch;
    float rate_ki_pitch;
    float rate_kd_pitch;
    float rate_kp_roll;
    float rate_ki_roll;
    float rate_kd_roll;
    float rate_kp_yaw;
    float rate_ki_yaw;
    float rate_kd_yaw;

    float z_kp;
    float z_ki;
    float z_kd;

    float d_filter_alpha;
    float output_min;
    float output_max;

    float nominal_battery_voltage;
    float voltage_scale_min;
    float voltage_scale_max;
} DroneControlParams;

void drone_control_default_params(DroneControlParams *params);
void drone_control_init(const DroneControlParams *params);
void drone_control_reset(void);

void drone_control_step(float dt,
                        float z_setpoint,
                        float z_meas,
                        float pitch_setpoint,
                        float roll_setpoint,
                        float yaw_setpoint,
                        float pitch_meas,
                        float roll_meas,
                        float yaw_meas,
                        float rate_pitch_meas,
                        float rate_roll_meas,
                        float rate_yaw_meas,
                        float battery_voltage,
                        float *motor_fl,
                        float *motor_fr,
                        float *motor_rl,
                        float *motor_rr);

#ifdef __cplusplus
}
#endif

#endif
