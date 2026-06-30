#include "tasks/control_task.h"
#include "tasks.h"
#include "data_structures.h"
#include "shared.h"
#include "tasks/fsm_task.h"
#include "hardware.h"
#include <queue.h>

#define PLACEHOLDER (0.0f)

namespace {
constexpr uint16_t PWM_TICKS_MIN = 819;
constexpr uint16_t PWM_TICKS_MAX = 1638;
constexpr float PWM_NORM_MIN = 0.0f;
constexpr float PWM_NORM_MAX = 1.0f;

float clamp01(float value)
{
    if (value < PWM_NORM_MIN) {
        return PWM_NORM_MIN;
    }
    if (value > PWM_NORM_MAX) {
        return PWM_NORM_MAX;
    }
    return value;
}

uint16_t mapNormalizedToTicks(float value)
{
    const float clamped = clamp01(value);
    return static_cast<uint16_t>(PWM_TICKS_MIN + (PWM_TICKS_MAX - PWM_TICKS_MIN) * clamped);
}

void applyMotorOutputs(const float motorOutputs[4])
{
    for (int i = 0; i < 4; ++i) {
        hardware_set_motor_throttle(static_cast<uint8_t>(i), mapNormalizedToTicks(motorOutputs[i]));
    }
}

void runArmTwitch(void)
{
    for (int pulse = 0; pulse < 3; ++pulse) {
        for (int i = 0; i < 4; ++i) {
            hardware_set_motor_throttle(static_cast<uint8_t>(i), mapNormalizedToTicks(0.12f));
        }
        vTaskDelay(pdMS_TO_TICKS(20));
        hardware_set_motors_idle();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

class P {
private:
    float Regeldiff_e;
    float p;
    float ProportionalVerstaerkung_Kp;
    float Fuehrungsgroesse_w;
    float Regelgroesse_x;

    float calculateP(float error) {
        return error * ProportionalVerstaerkung_Kp;
    }

public:
    float Regelausgangsgr_m;

    explicit P(float kp)
        : Regeldiff_e(0.0f), p(0.0f), ProportionalVerstaerkung_Kp(kp),
          Fuehrungsgroesse_w(0.0f), Regelgroesse_x(0.0f), Regelausgangsgr_m(0.0f) {}

    void step(float dt) {
        (void)dt;
        Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
        p = calculateP(Regeldiff_e);
        Regelausgangsgr_m = p;
    }

    void reset() {
        p = 0.0f;
    }

    void setInput(float w, float x) {
        Fuehrungsgroesse_w = w;
        Regelgroesse_x = x;
    }
};

class PID {
private:
    float Fuehrungsgroesse_w;
    float Regelgroesse_x;
    float Regeldiff_e;
    float last_x;
    float Integral_Ie;
    float rawDerivative_De;
    float filtertDerivative_fDe;
    float lastfiltertDerivative_lfDe;
    float p;
    float i;
    float d;
    bool saturated;
    float unclamped_m;
    float ProportionalVerstaerkung_Kp;
    float IntegralVerstaerkung_Ki;
    float DerivativeVerstaerkung_Kd;
    float D_FILETR_ALPHA;
    float OUT_MAX;
    float OUT_MIN;

    float calculateP(float error) {
        return error * ProportionalVerstaerkung_Kp;
    }

    float calculateI(float error, float dt) {
        if (!saturated) {
            Integral_Ie += error * dt;
        }
        return Integral_Ie * IntegralVerstaerkung_Ki;
    }

    float calculateD(float current_x, float dt) {
        rawDerivative_De = (current_x - last_x) / dt;
        filtertDerivative_fDe = D_FILETR_ALPHA * rawDerivative_De +
                                (1.0f - D_FILETR_ALPHA) * lastfiltertDerivative_lfDe;
        lastfiltertDerivative_lfDe = filtertDerivative_fDe;
        return filtertDerivative_fDe * DerivativeVerstaerkung_Kd;
    }

    void clampOutput() {
        unclamped_m = Regelausgangsgr_m;
        if (Regelausgangsgr_m > OUT_MAX) Regelausgangsgr_m = OUT_MAX;
        if (Regelausgangsgr_m < OUT_MIN) Regelausgangsgr_m = OUT_MIN;
        saturated = (Regelausgangsgr_m != unclamped_m);
    }

public:
    float Regelausgangsgr_m;

    PID(float kp, float ki, float kd, float alpha, float out_min = 0.0f, float out_max = 1.0f)
        : Fuehrungsgroesse_w(0.0f), Regelgroesse_x(0.0f), Regeldiff_e(0.0f),
          last_x(0.0f), Integral_Ie(0.0f), rawDerivative_De(0.0f),
          filtertDerivative_fDe(0.0f), lastfiltertDerivative_lfDe(0.0f),
          p(0.0f), i(0.0f), d(0.0f), saturated(false), unclamped_m(0.0f),
          ProportionalVerstaerkung_Kp(kp), IntegralVerstaerkung_Ki(ki),
          DerivativeVerstaerkung_Kd(kd), D_FILETR_ALPHA(alpha),
          OUT_MAX(out_max), OUT_MIN(out_min), Regelausgangsgr_m(0.0f) {}

    void step(float dt) {
        Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
        p = calculateP(Regeldiff_e);
        i = calculateI(Regeldiff_e, dt);
        d = calculateD(Regelgroesse_x, dt);
        Regelausgangsgr_m = p + i - d;
        clampOutput();
        last_x = Regelgroesse_x;
    }

    void reset() {
        Integral_Ie = 0.0f;
        last_x = Regelgroesse_x;
        lastfiltertDerivative_lfDe = 0.0f;
        saturated = false;
    }

    void setInput(float w, float x) {
        Fuehrungsgroesse_w = w;
        Regelgroesse_x = x;
    }
};
} // namespace

void TaskControl(void *pvParameters)
{
    static P pitchPID(0.5f);
    static P rollPID(0.5f);
    static P yawPID(0.4f);
    static PID ratePitchPID(1.0f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
    static PID rateRollPID(1.0f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
    static PID rateYawPID(1.0f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
    static PID ZspeedPID(1.0f, 0.2f, 0.0f, 0.7f, 0.0f, 1.0f);

    float DELTA_T = 0.0f;
    const float min_dt = 0.001f;

    int64_t last_us = 0;
    int64_t now_us;
    control_packet_t latest = {};
    bool has_latest = false;
    bool startup_twitch_done = false;

    for (;;)
    {
        now_us = esp_timer_get_time();

        if (last_us == 0) {
            last_us = now_us;
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        DELTA_T = (now_us - last_us) * 1e-6f;
        last_us = now_us;

        if (DELTA_T <= min_dt) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        control_packet_t pkt;
        while (xQueueReceive(inputQueue, &pkt, 0) == pdPASS) {
            latest = pkt;
            has_latest = true;
        }

        if (!startup_twitch_done) {
            runArmTwitch();
            startup_twitch_done = true;
        }

        float motorOutputsRaw[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float motorOutputs[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        if (fsm_armed && has_latest) {
            // TODO: Replace placeholder sensor inputs with real IMU/gyro feedback.
            // TODO: Tune the throttle and attitude PID gains against real flight data.
            // TODO: Add proper motor output scaling based on battery voltage and ESC range.
            ZspeedPID.setInput(latest.z, PLACEHOLDER);
            ZspeedPID.step(DELTA_T);
            float throttle = ZspeedPID.Regelausgangsgr_m;

            float currentPitch = PLACEHOLDER;
            float currentRoll = PLACEHOLDER;
            float currentYaw = PLACEHOLDER;
            float currentAltitude = PLACEHOLDER;

            EKFState_t ekfState = {};
            if (xQueuePeek(ekfQueue, &ekfState, 0) == pdPASS) {
                if (ekfState.valid_attitude) {
                    currentPitch = ekfState.pitch;
                    currentRoll = ekfState.roll;
                    currentYaw = ekfState.yaw;
                }
                if (ekfState.valid_altitude) {
                    currentAltitude = ekfState.altitude_m;
                }
            }

            pitchPID.setInput(PLACEHOLDER, currentPitch);
            rollPID.setInput(PLACEHOLDER, currentRoll);
            yawPID.setInput(PLACEHOLDER, currentYaw);

            pitchPID.step(DELTA_T);
            rollPID.step(DELTA_T);
            yawPID.step(DELTA_T);

            float currentRatePitch = PLACEHOLDER;
            float currentRateRoll = PLACEHOLDER;
            float currentRateYaw = PLACEHOLDER;

            ratePitchPID.setInput(pitchPID.Regelausgangsgr_m, currentRatePitch);
            rateRollPID.setInput(rollPID.Regelausgangsgr_m, currentRateRoll);
            rateYawPID.setInput(yawPID.Regelausgangsgr_m, currentRateYaw);

            ratePitchPID.step(DELTA_T);
            rateRollPID.step(DELTA_T);
            rateYawPID.step(DELTA_T);

            float motorCommandPitch = ratePitchPID.Regelausgangsgr_m;
            float motorCommandRoll = rateRollPID.Regelausgangsgr_m;
            float motorCommandYaw = rateYawPID.Regelausgangsgr_m;

            motorOutputsRaw[0] = throttle + motorCommandPitch + motorCommandRoll - motorCommandYaw;
            motorOutputsRaw[1] = throttle + motorCommandPitch - motorCommandRoll + motorCommandYaw;
            motorOutputsRaw[2] = throttle - motorCommandPitch - motorCommandRoll - motorCommandYaw;
            motorOutputsRaw[3] = throttle - motorCommandPitch + motorCommandRoll + motorCommandYaw;
        }

        for (int i = 0; i < 4; ++i) {
            motorOutputs[i] = clamp01(motorOutputsRaw[i]);
        }

        applyMotorOutputs(motorOutputs);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
