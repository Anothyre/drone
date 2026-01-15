#include "tasks/control_task.h"
#include "tasks.h"
#define PLACEHOLDER (0) 

float scaleOutput(float voltage);



class P{


    private:
        float Regeldiff_e ;
        float p; 
        float ProportionalVerstaerkung_Kp;   

    public:
        float Fuehrungsgroesse_w ;
        float Regelgroesse_x;
        float Regelausgangsgr_m;

    P(float kp){
        Fuehrungsgroesse_w =0.0f;
        Regelgroesse_x=0.0f;
        Regeldiff_e =0.0f;
        p=0.0f;
        Regelausgangsgr_m=0.0f;

        //Konstanten
        ProportionalVerstaerkung_Kp=kp;   
    }

     
  
    void pStep(){
        Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
        
        p = Regeldiff_e * ProportionalVerstaerkung_Kp;
        Regelausgangsgr_m = p;       
    
    }       
};


class PID{  
    //All we need i guess: https://www.linkedin.com/pulse/mastering-pid-control-drones-from-intuition-dr-ma-mohin-zbjqe/ 
 
    
    private:
        float Regeldiff_e ;
        float last_x;
        float Integral_Ie;
        float rawDerivative_De;
        float filtertDerivative_fDe;
        float lastfiltertDerivative_lfDe;
        float p;
        float i;
        float d; 
        bool  saturated ;
        float unclamped_m;
        float ProportionalVerstaerkung_Kp;  
        float IntegralVerstaerkung_Ki    ;
        float DerivativeVerstaerkung_Kd  ;
        float D_FILETR_ALPHA;          
        float OUT_MAX ; 
        float OUT_MIN ; 

        int64_t last_us = 0; 

    public:
        float Fuehrungsgroesse_w ;//TODO: make private with getter and setter (resetter)
        float Regelgroesse_x;
        float Regelausgangsgr_m;

    PID(float kp, float ki, float kd, float alpha,float out_min=0.0f, float out_max=1.0f){
        Fuehrungsgroesse_w =0.0f;
        Regelgroesse_x=0.0f;
        Regeldiff_e =0.0f;
        last_x=0.0f;
        Integral_Ie=0.0f;
        rawDerivative_De=0.0f;
        filtertDerivative_fDe=0.0f;
        lastfiltertDerivative_lfDe=0.0f;
        p=0.0f;
        i=0.0f;
        d=0.0f;
        Regelausgangsgr_m=0.0f;
        unclamped_m=0.0f;
        saturated = false;  

        //Konstanten
        ProportionalVerstaerkung_Kp=kp;   
        IntegralVerstaerkung_Ki    =ki;
        DerivativeVerstaerkung_Kd  =kd;
        D_FILETR_ALPHA =alpha;    
        OUT_MAX = out_max;
        OUT_MIN = out_min; 

        TickType_t last = DELTA_T;
    }

     
  
    void pidStep(DELTA_T){

        Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
        
        if(!saturated) 
        {
        Integral_Ie += Regeldiff_e * (DELTA_T);
        }

        rawDerivative_De = (Regelgroesse_x - last_x) / (DELTA_T);
        last_x     = Regelgroesse_x;
        filtertDerivative_fDe = D_FILETR_ALPHA * rawDerivative_De + (1 - D_FILETR_ALPHA) * lastfiltertDerivative_lfDe;
        //https://www.youtube.com/watch?v=HJ-C4Incgpw - die haben den ganzen fourje spaß imlementiert um unsr Alpha zu finden
        lastfiltertDerivative_lfDe = filtertDerivative_fDe;

        p = Regeldiff_e * ProportionalVerstaerkung_Kp;
        i = Integral_Ie * IntegralVerstaerkung_Ki;
        d = filtertDerivative_fDe * DerivativeVerstaerkung_Kd;
        Regelausgangsgr_m = p+i-d;//might fastforwarded 


                
        
        //TODO: better anti windup aproche needed?
        //gpt concept of anti windup

        unclamped_m = Regelausgangsgr_m;
        if (Regelausgangsgr_m > OUT_MAX) Regelausgangsgr_m = OUT_MAX;
        if (Regelausgangsgr_m < OUT_MIN) Regelausgangsgr_m = OUT_MIN;


        saturated = (Regelausgangsgr_m != unclamped_m);
        
    
    }       
    void resetIntegral() {
    Integral_Ie = 0.0f;
    }   

};



void TaskControl(void *pvParameters)
{
    // TODO:Read sensors 
    //TODO: think about dt handling

    //TODO: decide if fastforwarded and how  
    //TODO: might speed PID controller 
    //TODO#TOFU: Es gibt bessere Kotroller aufbauten als PID -https://www.preprints.org/manuscript/202509.1583

    //TODO: split in subtasks per axis and deside afterhow many rate call a normal call is needed 
     //TODO: tune all parameters
    static P pitchPID(1.0f);
    static P  rollPID(1.0f);
    static P   yawPID(1.0f);
    static PID ratePitchPID(1.0f, 0.0f, 0.0f, 0.7f,0.0f,1.0f);
    static PID rateRollPID(1.0f, 0.0f, 0.0f, 0.7f,0.0f,1.0f); 
    static PID rateYawPID(1.0f, 0.0f, 0.0f, 0.7f,0.0f,1.0f); 
    static PID ZspeedPID(1.0f, 0.2f, 0.0f, 0.7f,0.0f,1.0f); 
    
    float DELTA_T ;
    int64_t last_us = 0;
    int64_t now_us;

    for (;;)
    {
        now_us = esp_timer_get_time();
    
        // Schutz gegen den ersten Sprung
        if (last_us == 0) {
            last_us = now_us;
            return; // Erster Durchlauf überspringen
        }

        DELTA_T = (now_us - last_us) * 1e-6f;
        last_us = now_us;
        if(DELTA_T <= 0.00001f) {
           //TODO: handle too small DELTA_T and not with random magic numbers
        }



        ZspeedPID.Fuehrungsgroesse_w = PLACEHOLDER; //TODO: calc desired Z speed 
        ZspeedPID.Regelgroesse_x = PLACEHOLDER; //TODO: read current Z speed
        ZspeedPID.pidStep();
        float throttle = ZspeedPID.Regelausgangsgr_m; 


        // Read current attitude from EKF
        float currentPitch = PLACEHOLDER; //TODO: implement actual reading 
        float currentRoll  = PLACEHOLDER; 
        float currentYaw   = PLACEHOLDER; 

        // Update PID controllers for attitude
        pitchPID.Regelgroesse_x = currentPitch;
        rollPID.Regelgroesse_x  = currentRoll;
        yawPID.Regelgroesse_x   = currentYaw;


        // Update for Führungsgroesse_w from desired attitude commands
        pitchPID.Fuehrungsgroesse_w = PLACEHOLDER; 
        rollPID.Fuehrungsgroesse_w  = PLACEHOLDER; 
        yawPID.Fuehrungsgroesse_w   = PLACEHOLDER; 


        //TODO: Call less often
        pitchPID.pStep(DELTA_T);
        rollPID. pStep(DELTA_T);
        yawPID. pStep(DELTA_T);

        // Use attitude PID outputs as setpoints for rate controllers
        ratePitchPID.Fuehrungsgroesse_w = pitchPID.Regelausgangsgr_m;
        rateRollPID.Fuehrungsgroesse_w  = rollPID.Regelausgangsgr_m;
        rateYawPID.Fuehrungsgroesse_w   = yawPID.Regelausgangsgr_m;

        float currentRatePitch = PLACEHOLDER; //TODO: implement actual reading - if given use current rates from gyroscope

        float currentRateRoll  = PLACEHOLDER;
        float currentRateYaw   = PLACEHOLDER;

        // Update PID controllers for rates
        ratePitchPID.Regelgroesse_x = currentRatePitch;
        rateRollPID.Regelgroesse_x  = currentRateRoll;
        rateYawPID.Regelgroesse_x   = currentRateYaw;  
        ratePitchPID.pidStep(DELTA_T);
        rateRollPID.pidStep(DELTA_T);
        rateYawPID.pidStep(DELTA_T);
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
        
        //TODO: clamp motoers and skale 
        //TODO: output PWM
        //TODO:vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz 
}

    //const TickType_t xDelay = pdMS_TO_TICKS(2); // 500Hz control loop
}
  
float scaleOutput(float voltage) {
    return 42;   //TODO: implement actual scaling based on voltage table
    
}

   
