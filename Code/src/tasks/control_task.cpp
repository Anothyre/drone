#include "tasks/control_task.h"
#include "tasks.h"

#define PLACEHOLDER (0) //TODO: implement actual reading 
#define DELTA_T (1)// TODO: adjust according to task frequency
#define ProportionalVerstaerkung_Kp (1.0f)
#define IntegralVerstaerkung_Ki (1.0f)
#define DerivativeVerstaerkung_Kd (1.0f)

#define D_FILETR_ALPHA 0.7//TODO: tune bzw. ideet calculate



//All we need i guess: https://www.linkedin.com/pulse/mastering-pid-control-drones-from-intuition-dr-ma-mohin-zbjqe/ 

void TaskControl(void *pvParameters)
{
    TickType_t last = xTaskGetTickCount();

       //TODO: Solve trogh structs

        //static because multiple calls
        static float Fuehrungsgroesse_w =0.0f;
        static float Regelgroesse_x=0.0f;
        static float Regeldiff_e =0.0f;
        static float last_x=0.0f;
        static float Integral_Ie=0.0f;
        static float rawDerivative_De=0.0f;
        static float filtertDerivative_De=0.0f;
        static float lastfiltertDerivative_De=0.0f;
        static float p;
        static float i;
        static float d;
        static float Regelausgangsgr_m;

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2));
        //TODO: for each loop think if I and D are needed


        // read attitude from ekf
        // run PID  o.AE.
        // TODO; Concept - TOGETHER
        //TODO: decide if fastforwarded 
        //TODO: cascade also rate control?




        Fuehrungsgroesse_w = PLACEHOLDER;
        Regelgroesse_x = PLACEHOLDER;
        Regeldiff_e = Fuehrungsgroesse_w - Regelgroesse_x;
        Integral_Ie += Regeldiff_e * (DELTA_T);
        // TODO:Anti-windup
       




        rawDerivative_De = (Regelgroesse_x - last_x) / (DELTA_T);
        last_x     = Regelgroesse_x;  
        filtertDerivative_De = D_FILETR_ALPHA * rawDerivative_De + (1 - D_FILETR_ALPHA) * lastfiltertDerivative_De;
        //https://www.youtube.com/watch?v=HJ-C4Incgpw - die haben den ganzen fourje spaß imlementiert um unsr Alpha zu finden
        lastfiltertDerivative_De = filtertDerivative_De;




        p = Regeldiff_e * ProportionalVerstaerkung_Kp;
        i = Integral_Ie * IntegralVerstaerkung_Ki;
        d = filtertDerivative_De * DerivativeVerstaerkung_Kd;
        Regelausgangsgr_m = p+i+d;//might fastforwarded 

  



       
        // mix motors

        // output PWM
        vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz // TODO: adjust for all tasks
    }
}