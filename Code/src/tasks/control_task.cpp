#include "tasks/control_task.h"
#include "tasks.h"

#define PLACEHOLDER (0) //TODO: implement actual reading 

struct PID{
       //static because multiple calls
        static float Fuehrungsgroesse_w ;
        static float Regelgroesse_x;
        static float Regeldiff_e ;
        static float last_x;
        static float Integral_Ie;
        static float rawDerivative_De;
        static float filtertDerivative_fDe;
        static float lastfiltertDerivative_lfDe;
        static float p;
        static float i;
        static float d;
        static float Regelausgangsgr_m;
        static bool  saturated ; 
        static float DELTA_T;                     // TODO: adjust according to task frequency
        static float ProportionalVerstaerkung_Kp;   //tune
        static float IntegralVerstaerkung_Ki    ;
        static float DerivativeVerstaerkung_Kd  ;
        static float D_FILETR_ALPHA;           //TODO: tune bzw. ideet calculate
 
};

//All we need i guess: https://www.linkedin.com/pulse/mastering-pid-control-drones-from-intuition-dr-ma-mohin-zbjqe/ 

void TaskControl(void *pvParameters)
{
    //   DEMO PID 
        PID demoPID;

        demoPID.Fuehrungsgroesse_w =0.0f;
        demoPID.Regelgroesse_x=0.0f;
        demoPID.Regeldiff_e =0.0f;
        demoPID.last_x=0.0f;
        demoPID.Integral_Ie=0.0f;
        demoPID.rawDerivative_De=0.0f;
        demoPID.filtertDerivative_fDe=0.0f;
        demoPID.lastfiltertDerivative_lfDe=0.0f;
        demoPID.p;
        demoPID.i;
        demoPID.d;
        demoPID.Regelausgangsgr_m;
        demoPID.saturated = false;  

        //Konstanten
        demoPID. DELTA_T                    =0.002f; // TODO: adjust according to task frequency
        demoPID. ProportionalVerstaerkung_Kp=0.0f;   
        demoPID. IntegralVerstaerkung_Ki    =0.0f;
        demoPID. DerivativeVerstaerkung_Kd  =0.0f;
        demoPID.D_FILETR_ALPHA = 0.7;         


        TickType_t last = xTaskGetTickCount();
        int64_t last_us = 0;//TODO: initialize properly

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2)); //TODO: anscheinded mit vTaskDelayUntil zusammen doof

        int64_t now_us = esp_timer_get_time();
        demoPID.DELTA_T = (now_us - last_us) * 1e-6f;
        last_us = now_us;
        //TODO: for each loop think if I and D are needed
        // read attitude from ekf
        // run PID  o.AE.
        // TODO; Concept - TOGETHER
        //TODO: decide if fastforwarded 
        //TODO: cascade also rate control? - split into different tasks

        demoPID.Fuehrungsgroesse_w = PLACEHOLDER;//TODO: implement actual reading and writing
        demoPID.Regelgroesse_x = PLACEHOLDER;
        demoPID.Regeldiff_e = demoPID.Fuehrungsgroesse_w - demoPID.Regelgroesse_x;
        demoPID.saturated = false; //TODO: implement saturation detection

        //TODO: better anti windup aproche
        if(!demoPID.saturated) //TODO: check saturation condition  -:https://youtu.be/NVLXCwc8HzM?si=FpmdmcUAy8FntkJC&t=386
        {
        demoPID.Integral_Ie += demoPID.Regeldiff_e * (demoPID.DELTA_T);
        }

        demoPID.rawDerivative_De = (demoPID.Regelgroesse_x - demoPID.last_x) / (demoPID.DELTA_T);
        demoPID.last_x     = demoPID.Regelgroesse_x;  
        demoPID.filtertDerivative_fDe = demoPID.D_FILETR_ALPHA * demoPID.rawDerivative_De + (1 - demoPID.D_FILETR_ALPHA) * demoPID.lastfiltertDerivative_lfDe;
        //https://www.youtube.com/watch?v=HJ-C4Incgpw - die haben den ganzen fourje spaß imlementiert um unsr Alpha zu finden
        demoPID.lastfiltertDerivative_lfDe = demoPID.filtertDerivative_fDe;

        demoPID.p = demoPID.Regeldiff_e * demoPID.ProportionalVerstaerkung_Kp;
        demoPID.i = demoPID.Integral_Ie * demoPID.IntegralVerstaerkung_Ki;
        demoPID.d = demoPID.filtertDerivative_fDe * demoPID.DerivativeVerstaerkung_Kd;
        demoPID.Regelausgangsgr_m = demoPID.p+demoPID.i+demoPID.d;//might fastforwarded 

        
        //gpt concept of anti windup
        // out = clamp(out, MIN_CMD, MAX_CMD);
        // saturated = (out != unclamped_out);
        
        // mix motors - probaly just default quadcopter x config

        // output PWM
        vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz // TODO: adjust for all tasks
    }
}
