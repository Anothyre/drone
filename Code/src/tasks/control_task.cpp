#include "tasks/control_task.h"
#include "tasks.h"





#define PLACEHOLDER (0) //TODO: implement actual reading 



// struct PIDvars{
//        //static because multiple calls
//         static float Fuehrungsgroesse_w ;
//         static float Regelgroesse_x;
//         static float Regeldiff_e ;
//         static float last_x;
//         static float Integral_Ie;
//         static float rawDerivative_De;
//         static float filtertDerivative_fDe;
//         static float lastfiltertDerivative_lfDe;
//         static float p;
//         static float i;
//         static float d;
//         static float Regelausgangsgr_m;
//         static bool  saturated ; 
//         static float DELTA_T;                     // TOD: adjust according to task frequency
//         static float ProportionalVerstaerkung_Kp;   //tune
//         static float IntegralVerstaerkung_Ki    ;
//         static float DerivativeVerstaerkung_Kd  ;
//         static float D_FILETR_ALPHA;           //TOD: tune bzw. ideet calculate
 
// };

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
        float DELTA_T;                     // TODO: adjust according to task frequency
        float ProportionalVerstaerkung_Kp;   //tune
        float IntegralVerstaerkung_Ki    ;
        float DerivativeVerstaerkung_Kd  ;
        float D_FILETR_ALPHA;           //TODO: tune bzw. ideet calculate
        int64_t last_us = 0;//TODO: initialize properly    

    public:
        float Fuehrungsgroesse_w ;//TODO: make private with getter and setter (resetter)
        float Regelgroesse_x;
        float Regelausgangsgr_m;

    PID(float kp, float ki, float kd, float alpha){
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
        saturated = false;  

        //Konstanten
        DELTA_T                    =0.002f; // TODO: adjust according to task frequency
        ProportionalVerstaerkung_Kp=kp;   
        IntegralVerstaerkung_Ki    =ki;
        DerivativeVerstaerkung_Kd  =kd;
        D_FILETR_ALPHA =alpha;    

        TickType_t last = DELTA_T;
        
    }

     
  
    void pidStep(){
        int64_t now_us = esp_timer_get_time();
    
        // Schutz gegen den ersten Sprung
        if (last_us == 0) {
            last_us = now_us;
            return; // Erster Durchlauf überspringen
        }

        DELTA_T = (now_us - last_us) * 1e-6f;
        last_us = now_us;
        if(DELTA_T <= 0.00001f) {
           //TODO: handle too small DELTA_T
        }

        Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
        
        saturated = false; //TODO: implement saturation detection
        //TODO: better anti windup aproche
        if(!saturated) //TODO: check saturation condition  -:https://youtu.be/NVLXCwc8HzM?si=FpmdmcUAy8FntkJC&t=386
        {
        Integral_Ie += Regeldiff_e * (DELTA_T);
        }

        rawDerivative_De = (Regeldiff_e - last_x) / (DELTA_T);
        last_x     = Regelgroesse_x;  
        filtertDerivative_fDe = D_FILETR_ALPHA * rawDerivative_De + (1 - D_FILETR_ALPHA) * lastfiltertDerivative_lfDe;
        //https://www.youtube.com/watch?v=HJ-C4Incgpw - die haben den ganzen fourje spaß imlementiert um unsr Alpha zu finden
        lastfiltertDerivative_lfDe = filtertDerivative_fDe;

        p = Regeldiff_e * ProportionalVerstaerkung_Kp;
        i = Integral_Ie * IntegralVerstaerkung_Ki;
        d = filtertDerivative_fDe * DerivativeVerstaerkung_Kd;
        Regelausgangsgr_m = p+i-d;//might fastforwarded 

        
        //gpt concept of anti windup
        // out = clamp(out, MIN_CMD, MAX_CMD);
        // saturated = (out != unclamped_out);
        
        int64_t now_us = esp_timer_get_time();
        DELTA_T = (now_us - last_us) * 1e-6f;
        last_us = now_us;
    
    }       

};







void TaskControl(void *pvParameters)
{
    //TODO: for each loop think if I and D are needed
   // read attitude from ek

   // TODO; Concept - TOGETHER
   //TODO: decide if fastforwarded 
   //TODO: cascade / rate control implementation



    //TODO: split in subtasks 
    PID pitchPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    PID rollPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    PID yawPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
        //TODO: ==> in rate controller schieben
    PID ratePitchPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    PID rateRollPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    PID rateYawPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters


   


    // //   DEMO PID 
    //     PIDvars demoPID;

    //     Fuehrungsgroesse_w =0.0f;
    //     Regelgroesse_x=0.0f;
    //     Regeldiff_e =0.0f;
    //     last_x=0.0f;
    //     Integral_Ie=0.0f;
    //     rawDerivative_De=0.0f;
    //     filtertDerivative_fDe=0.0f;
    //     lastfiltertDerivative_lfDe=0.0f;
    //     p;
    //     i;
    //     d;
    //     Regelausgangsgr_m;
    //     saturated = false;  

    //     //Konstanten
    //      DELTA_T                    =0.002f; // TODO: adjust according to task frequency
    //      ProportionalVerstaerkung_Kp=0.0f;   
    //      IntegralVerstaerkung_Ki    =0.0f;
    //      DerivativeVerstaerkung_Kd  =0.0f;
    //     D_FILETR_ALPHA = 0.7;         


    //     TickType_t last = xTaskGetTickCount();
    //     int64_t last_us = 0;//TODO: initialize properly

    // for (;;)
    // {
    //     ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2)); //TODO: anscheinded mit vTaskDelayUntil zusammen doof

    //     int64_t now_us = esp_timer_get_time();
    //     DELTA_T = (now_us - last_us) * 1e-6f;
    //     last_us = now_us;
    //     

    //     Fuehrungsgroesse_w = PLACEHOLDER;//TODO: implement actual reading and writing
    //     Regelgroesse_x = PLACEHOLDER;
    //     Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
    //     saturated = false; //TODO: implement saturation detection

    //     //TODO: better anti windup aproche
    //     if(!saturated) //TODO: check saturation condition  -:https://youtu.be/NVLXCwc8HzM?si=FpmdmcUAy8FntkJC&t=386
    //     {
    //     Integral_Ie += Regeldiff_e * (DELTA_T);
    //     }

    //     rawDerivative_De = (Regelgroesse_x - last_x) / (DELTA_T);
    //     last_x     = Regelgroesse_x;  
    //     filtertDerivative_fDe = D_FILETR_ALPHA * rawDerivative_De + (1 - D_FILETR_ALPHA) * lastfiltertDerivative_lfDe;
    //     //https://www.youtube.com/watch?v=HJ-C4Incgpw - die haben den ganzen fourje spaß imlementiert um unsr Alpha zu finden
    //     lastfiltertDerivative_lfDe = filtertDerivative_fDe;

    //     p = Regeldiff_e * ProportionalVerstaerkung_Kp;
    //     i = Integral_Ie * IntegralVerstaerkung_Ki;
    //     d = filtertDerivative_fDe * DerivativeVerstaerkung_Kd;
    //     Regelausgangsgr_m = p+i+d;//might fastforwarded 

        
        //gpt concept of anti windup
        // out = clamp(out, MIN_CMD, MAX_CMD);
        // saturated = (out != unclamped_out);
        
        // mix motors - probaly just default quadcopter x config
            // float throttle = rc_throttle; // Basisgas vom Stick

           // Beispiel Quad X - by gemini ahh bard
            // float motor1 = throttle + pid_out_pitch + pid_out_roll - pid_out_yaw; // Vorne Links (CW)
            // float motor2 = throttle + pid_out_pitch - pid_out_roll + pid_out_yaw; // Vorne Rechts (CCW)
            // float motor3 = throttle - pid_out_pitch - pid_out_roll - pid_out_yaw; // Hinten Links (CCW)
            // float motor4 = throttle - pid_out_pitch + pid_out_roll + pid_out_yaw; // Hinten Rechts (CW)

        // output PWM
        //vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz // TODO: adjust for all tasks
}
