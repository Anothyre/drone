#include "control_task_simulink.h"

namespace {

float clampf(float value, float min_value, float max_value) {
    if (value > max_value) {
        return max_value;
    }
    if (value < min_value) {
        return min_value;
    }
    return value;
}

class PController {
public:
    PController() : kp_(0.0f), output_(0.0f) {}

    void configure(float kp) { kp_ = kp; }

    float step(float setpoint, float measurement) {
        output_ = (setpoint - measurement) * kp_;
        return output_;
    }

    void reset() { output_ = 0.0f; }

private:
    float kp_;
    float output_;
};

class PIDController {
public:
    PIDController()
        : kp_(0.0f),
          ki_(0.0f),
          kd_(0.0f),
          alpha_(0.7f),
          out_min_(0.0f),
          out_max_(1.0f),
          last_measurement_(0.0f),
          integral_(0.0f),
          last_filtered_derivative_(0.0f),
          saturated_(false) {}

    void configure(float kp, float ki, float kd, float alpha, float out_min, float out_max) {
        kp_ = kp;
        ki_ = ki;
        kd_ = kd;
        alpha_ = clampf(alpha, 0.0f, 1.0f);
        out_min_ = out_min;
        out_max_ = out_max;
    }

    float step(float dt, float setpoint, float measurement) {
        const float safe_dt = dt > 1.0e-6f ? dt : 1.0e-6f;
        const float error = setpoint - measurement;

        const float p = error * kp_;

        if (!saturated_) {
            integral_ += error * safe_dt;
        }
        const float i = integral_ * ki_;

        const float raw_derivative = (measurement - last_measurement_) / safe_dt;
        const float filtered_derivative =
            (alpha_ * raw_derivative) + ((1.0f - alpha_) * last_filtered_derivative_);
        const float d = filtered_derivative * kd_;

        float output = p + i - d;
        const float unclamped = output;
        output = clampf(output, out_min_, out_max_);
        saturated_ = (output != unclamped);

        last_measurement_ = measurement;
        last_filtered_derivative_ = filtered_derivative;
        return output;
    }

    void reset(float measurement_for_state) {
        last_measurement_ = measurement_for_state;
        integral_ = 0.0f;
        last_filtered_derivative_ = 0.0f;
        saturated_ = false;
    }

private:
    float kp_;
    float ki_;
    float kd_;
    float alpha_;
    float out_min_;
    float out_max_;

    float last_measurement_;
    float integral_;
    float last_filtered_derivative_;
    bool saturated_;
};

struct ControlState {
    bool initialized;
    DroneControlParams params;
    PController pitch_p;
    PController roll_p;
    PController yaw_p;
    PIDController rate_pitch_pid;
    PIDController rate_roll_pid;
    PIDController rate_yaw_pid;
    PIDController z_speed_pid;
};

ControlState g_state = {};

float compute_voltage_scale(float battery_voltage, const DroneControlParams &params) {
    if (battery_voltage <= 1.0e-6f) {
        return 1.0f;
    }
    const float raw_scale = params.nominal_battery_voltage / battery_voltage;
    return clampf(raw_scale, params.voltage_scale_min, params.voltage_scale_max);
}

}  // namespace

extern "C" void drone_control_default_params(DroneControlParams *params) {
    if (params == 0) {
        return;
    }

    params->attitude_kp_pitch = 0.5f;
    params->attitude_kp_roll = 0.5f;
    params->attitude_kp_yaw = 0.4f;

    params->rate_kp_pitch = 1.0f;
    params->rate_ki_pitch = 0.0f;
    params->rate_kd_pitch = 0.0f;
    params->rate_kp_roll = 1.0f;
    params->rate_ki_roll = 0.0f;
    params->rate_kd_roll = 0.0f;
    params->rate_kp_yaw = 1.0f;
    params->rate_ki_yaw = 0.0f;
    params->rate_kd_yaw = 0.0f;

    params->z_kp = 1.0f;
    params->z_ki = 0.2f;
    params->z_kd = 0.0f;

    params->d_filter_alpha = 0.7f;
    params->output_min = 0.0f;
    params->output_max = 1.0f;

    params->nominal_battery_voltage = 12.0f;
    params->voltage_scale_min = 0.8f;
    params->voltage_scale_max = 1.2f;
}

extern "C" void drone_control_init(const DroneControlParams *params) {
    DroneControlParams defaults = {};
    drone_control_default_params(&defaults);

    g_state.params = (params != 0) ? *params : defaults;

    g_state.pitch_p.configure(g_state.params.attitude_kp_pitch);
    g_state.roll_p.configure(g_state.params.attitude_kp_roll);
    g_state.yaw_p.configure(g_state.params.attitude_kp_yaw);

    g_state.rate_pitch_pid.configure(g_state.params.rate_kp_pitch,
                                     g_state.params.rate_ki_pitch,
                                     g_state.params.rate_kd_pitch,
                                     g_state.params.d_filter_alpha,
                                     g_state.params.output_min,
                                     g_state.params.output_max);
    g_state.rate_roll_pid.configure(g_state.params.rate_kp_roll,
                                    g_state.params.rate_ki_roll,
                                    g_state.params.rate_kd_roll,
                                    g_state.params.d_filter_alpha,
                                    g_state.params.output_min,
                                    g_state.params.output_max);
    g_state.rate_yaw_pid.configure(g_state.params.rate_kp_yaw,
                                   g_state.params.rate_ki_yaw,
                                   g_state.params.rate_kd_yaw,
                                   g_state.params.d_filter_alpha,
                                   g_state.params.output_min,
                                   g_state.params.output_max);
    g_state.z_speed_pid.configure(g_state.params.z_kp,
                                  g_state.params.z_ki,
                                  g_state.params.z_kd,
                                  g_state.params.d_filter_alpha,
                                  g_state.params.output_min,
                                  g_state.params.output_max);

    g_state.pitch_p.reset();
    g_state.roll_p.reset();
    g_state.yaw_p.reset();
    g_state.rate_pitch_pid.reset(0.0f);
    g_state.rate_roll_pid.reset(0.0f);
    g_state.rate_yaw_pid.reset(0.0f);
    g_state.z_speed_pid.reset(0.0f);

    g_state.initialized = true;
}

extern "C" void drone_control_reset(void) {
    if (!g_state.initialized) {
        return;
    }

    g_state.pitch_p.reset();
    g_state.roll_p.reset();
    g_state.yaw_p.reset();
    g_state.rate_pitch_pid.reset(0.0f);
    g_state.rate_roll_pid.reset(0.0f);
    g_state.rate_yaw_pid.reset(0.0f);
    g_state.z_speed_pid.reset(0.0f);
}

extern "C" void drone_control_step(float dt,
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
                                   float *motor_rr) {
    if (motor_fl == 0 || motor_fr == 0 || motor_rl == 0 || motor_rr == 0) {
        return;
    }

    if (!g_state.initialized) {
        drone_control_init(0);
    }

    const float safe_dt = dt > 1.0e-6f ? dt : 1.0e-6f;

    const float throttle = g_state.z_speed_pid.step(safe_dt, z_setpoint, z_meas);

    const float pitch_rate_setpoint = g_state.pitch_p.step(pitch_setpoint, pitch_meas);
    const float roll_rate_setpoint = g_state.roll_p.step(roll_setpoint, roll_meas);
    const float yaw_rate_setpoint = g_state.yaw_p.step(yaw_setpoint, yaw_meas);

    const float pitch_cmd = g_state.rate_pitch_pid.step(safe_dt, pitch_rate_setpoint, rate_pitch_meas);
    const float roll_cmd = g_state.rate_roll_pid.step(safe_dt, roll_rate_setpoint, rate_roll_meas);
    const float yaw_cmd = g_state.rate_yaw_pid.step(safe_dt, yaw_rate_setpoint, rate_yaw_meas);

    float raw_fl = throttle + pitch_cmd + roll_cmd - yaw_cmd;
    float raw_fr = throttle + pitch_cmd - roll_cmd + yaw_cmd;
    float raw_rl = throttle - pitch_cmd - roll_cmd - yaw_cmd;
    float raw_rr = throttle - pitch_cmd + roll_cmd + yaw_cmd;

    const float scale = compute_voltage_scale(battery_voltage, g_state.params);

    *motor_fl = clampf(raw_fl * scale, 0.0f, 1.0f);
    *motor_fr = clampf(raw_fr * scale, 0.0f, 1.0f);
    *motor_rl = clampf(raw_rl * scale, 0.0f, 1.0f);
    *motor_rr = clampf(raw_rr * scale, 0.0f, 1.0f);
}
