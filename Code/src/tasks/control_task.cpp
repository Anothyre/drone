#include "tasks/control_task.h"
#include "tasks.h"
#include "data_structures.h"
#include "shared.h"
#define PLACEHOLDER (0) 

float scaleOutput(float voltage);


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

        P(float kp)
            : Regeldiff_e(0.0f), p(0.0f), ProportionalVerstaerkung_Kp(kp),
              Fuehrungsgroesse_w(0.0f), Regelgroesse_x(0.0f), Regelausgangsgr_m(0.0f) {}

        void step(float dt) {
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

        void getOutput(float &out) {
            out = Regelausgangsgr_m;
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

        void getOutput(float &out) {
            out = Regelausgangsgr_m;
        }
};



void TaskControl(void *pvParameters)
{
    // TODO:Read sensors 
    //TODO:THINK about dt handling - shold be done but discuss

    //TODO:THINK: decide if fastforwarded and how  
    //TODO:THINK: might speed PID controller 
    //TODO#TOFU: Es gibt bessere Kotroller aufbauten als PID -https://www.preprints.org/manuscript/202509.1583

    //TODO: split in subtasks per axis and deside afterhow many rate call a normal call is needed 
     //TODO: TUNE all parameters
    static P pitchPID(0.5f);
    static P  rollPID(0.5f);
    static P   yawPID(0.4f);
    static PID ratePitchPID(1.0f, 0.0f, 0.0f, 0.7f,0.0f,1.0f);
    static PID rateRollPID(1.0f, 0.0f, 0.0f, 0.7f,0.0f,1.0f); 
    static PID rateYawPID(1.0f, 0.0f, 0.0f, 0.7f,0.0f,1.0f); 
    static PID ZspeedPID(1.0f, 0.2f, 0.0f, 0.7f,0.0f,1.0f); 

        
    float DELTA_T ;
    const int64_t min_dt = 0.00001f; //1ms minimum dt

    int64_t last_us = 0;
    int64_t now_us;

    for (;;)
    {
         
        now_us = esp_timer_get_time();
    
        // Schutz gegen den ersten Sprung
        if (last_us == 0) {
            last_us = now_us;
            continue; // Erster Durchlauf überspringen
        }

        DELTA_T = (now_us - last_us) * 1e-6f;
         if(DELTA_T <= min_dt) {
            
            continue;
        }
        last_us = now_us;//wait until bigger than min dt
        control_packet_t pkt, latest;
        bool has_new = false;
        while (xQueueReceive(inputQueue, &pkt, 0) == pdPASS) {
        latest = pkt;      // immer überschreiben -> neuestes bleibt
        has_new = true;
        }       


        ZspeedPID.setInput(latest.z, PLACEHOLDER);//TODO: what has to go in the PID actually ?
        ZspeedPID.step(DELTA_T);
        float throttle = ZspeedPID.Regelausgangsgr_m; 


        // Read current attitude from EKF
        float currentPitch = PLACEHOLDER; //TODO: implement actual reading 
        float currentRoll  = PLACEHOLDER; 
        float currentYaw   = PLACEHOLDER; 

        // Update PID controllers for attitude
        pitchPID.setInput(PLACEHOLDER, currentPitch);
        rollPID.setInput(PLACEHOLDER, currentRoll);
        yawPID.setInput(PLACEHOLDER, currentYaw);

        // TODO: Call less often
        pitchPID.step(DELTA_T);
        rollPID.step(DELTA_T);
        yawPID.step(DELTA_T);

        // Read current rates from gyroscope
        float currentRatePitch = PLACEHOLDER; //TODO: implement actual reading
        float currentRateRoll  = PLACEHOLDER;
        float currentRateYaw   = PLACEHOLDER;

        // Use attitude PID outputs as setpoints for rate controllers
        ratePitchPID.setInput(pitchPID.Regelausgangsgr_m, currentRatePitch);
        rateRollPID.setInput(rollPID.Regelausgangsgr_m, currentRateRoll);
        rateYawPID.setInput(yawPID.Regelausgangsgr_m, currentRateYaw);

        // Update PID controllers for rates
        ratePitchPID.step(DELTA_T);
        rateRollPID.step(DELTA_T);
        rateYawPID.step(DELTA_T);
        // Compute motor commands based on rate PID outputs
        float motorCommandPitch = ratePitchPID.Regelausgangsgr_m;
        float motorCommandRoll  = rateRollPID.Regelausgangsgr_m;
        float motorCommandYaw   = rateYawPID.Regelausgangsgr_m;


        float motorOutputsRaw[4];

           // Beispiel Quad X - by gemini ahh bard -should be fine
         motorOutputsRaw[0] = throttle + motorCommandPitch + motorCommandRoll - motorCommandYaw; // Vorne Links (CW)
         motorOutputsRaw[1] = throttle + motorCommandPitch - motorCommandRoll + motorCommandYaw; // Vorne Rechts (CCW)
         motorOutputsRaw[2] = throttle - motorCommandPitch - motorCommandRoll - motorCommandYaw; // Hinten Links (CCW)
         motorOutputsRaw[3] = throttle - motorCommandPitch + motorCommandRoll + motorCommandYaw; // Hinten Rechts (CW)


        float voltage = PLACEHOLDER; //TODO: implement actual reading
        float scalingFactor =  scaleOutput(voltage);

        float motorOutputs[4];
        for (int i = 0; i < 4; i++) {
            motorOutputs[i] = motorOutputsRaw[i] * scalingFactor;        
            if (motorOutputs[i] > 1.0f) motorOutputs[i] = 1.0f;
            if (motorOutputs[i] < 0.0f) motorOutputs[i] = 0.0f;
        }
        
        // TODO: clamp motors and scale 
        // TODO: output PWM
        // TODO: vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz 
    }
}
  
float scaleOutput(float voltage) {
    //TODO: implement actual scaling based on voltage table
    return 42;
}

   
