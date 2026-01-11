#include "tasks/control_task.h"
#include "tasks.h"
#define PLACEHOLDER (0) //TODO: implement actual reading 

float scaleOutput(float voltage);


class PID{  
    //All we need i guess: https://www.linkedin.com/pulse/mastering-pid-control-drones-from-intuition-dr-ma-mohin-zbjqe/ 
 
    
    private:
        float Regeldiff_e ;
        float last_e;
        float Integral_Ie;
        float rawDerivative_De;
        float filtertDerivative_fDe;
        float lastfiltertDerivative_lfDe;
        float p;
        float i;
        float d; 
        bool  saturated ; 
        float DELTA_T;                     // TODO: adjust according to task frequency
        float ProportionalVerstaerkung_Kp;   //TODO: tune
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
        last_e=0.0f;
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
        int64_t now_us = esp_timer_get_time();//TODO:move out of loop for cascade
    
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
        //TODO: better anti windup aproche needed?
        if(!saturated) //TODO: check saturation condition  -:https://youtu.be/NVLXCwc8HzM?si=FpmdmcUAy8FntkJC&t=386
        {
        Integral_Ie += Regeldiff_e * (DELTA_T);
        }

        rawDerivative_De = (Regeldiff_e - last_e) / (DELTA_T);
        last_e     = Regeldiff_e;
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
        
    
    }       

};



void TaskControl(void *pvParameters)
{
    //TODO: for each loop think if I and D are needed
   // read attitude from ek

   // TODO; Concept - TOGETHER
   //TODO: decide if fastforwarded 
   //TODO: cascade / rate control implementation
    //TODO: might speed PID controller 

    //TODO: split in subtasks 
    static PID pitchPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    static PID rollPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    static PID yawPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
        //TODO: Ausgaben ==> in rate controller schieben
    static PID ratePitchPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    static PID rateRollPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters
    static PID rateYawPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters

    static PID ZspeedPID(1.0f, 0.0f, 0.0f, 0.7f); //TODO: tune parameters


    for (;;)
    {

        ZspeedPID.Fuehrungsgroesse_w = PLACEHOLDER; //TODO: set desired Z speed 
        ZspeedPID.Regelgroesse_x = PLACEHOLDER; //TODO: read current Z speed from EKF or barometer
        ZspeedPID.pidStep();
        float throttle = ZspeedPID.Regelausgangsgr_m; 


        // Read current attitude from EKF
        float currentPitch = PLACEHOLDER; //TODO: implement actual reading
        float currentRoll  = PLACEHOLDER; //TODO: implement actual reading
        float currentYaw   = PLACEHOLDER; //TODO: implement actual reading

        // Update PID controllers for attitude
        pitchPID.Regelgroesse_x = currentPitch;
        rollPID.Regelgroesse_x  = currentRoll;
        yawPID.Regelgroesse_x   = currentYaw;


        // Update for Führungsgroesse_w from desired attitude commands
        pitchPID.Fuehrungsgroesse_w = PLACEHOLDER; //TODO:
        rollPID.Fuehrungsgroesse_w  = PLACEHOLDER; //TODO:
        yawPID.Fuehrungsgroesse_w   = PLACEHOLDER; //TODO 


        
        pitchPID.pidStep();
        rollPID.pidStep();
        yawPID.pidStep();

        // Use attitude PID outputs as setpoints for rate controllers
        ratePitchPID.Fuehrungsgroesse_w = pitchPID.Regelausgangsgr_m;
        rateRollPID.Fuehrungsgroesse_w  = rollPID.Regelausgangsgr_m;
        rateYawPID.Fuehrungsgroesse_w   = yawPID.Regelausgangsgr_m;

        // Read current rates from gyroscope
        float currentRatePitch = PLACEHOLDER; //TODO: implement actual reading
        float currentRateRoll  = PLACEHOLDER; //TODO: implement actual reading
        float currentRateYaw   = PLACEHOLDER; //TODO: implement actual reading

        // Update PID controllers for rates
        ratePitchPID.Regelgroesse_x = currentRatePitch;
        rateRollPID.Regelgroesse_x  = currentRateRoll;
        rateYawPID.Regelgroesse_x   = currentRateYaw;  
        ratePitchPID.pidStep();
        rateRollPID.pidStep();
        rateYawPID.pidStep();
        // Compute motor commands based on rate PID outputs
        float motorCommandPitch = ratePitchPID.Regelausgangsgr_m;
        float motorCommandRoll  = rateRollPID.Regelausgangsgr_m;
        float motorCommandYaw   = rateYawPID.Regelausgangsgr_m;


         // mix motors - probaly just default quadcopter x config
            // float throttle = rc_throttle; // Basisgas vom Stick


        float motorOutputsRaw[4];

           // Beispiel Quad X - by gemini ahh bard
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
        
        
        // output PWM
        //vTaskDelayUntil(&last, pdMS_TO_TICKS(2)); // ~500Hz // TODO: adjust for all tasks
    }
}

    const TickType_t xDelay = pdMS_TO_TICKS(2); // 500Hz control loop

  
float scaleOutput(float voltage) {
    return 42;   //TODO: implement actual scaling based on voltage table
    
}

   


/*CHATER LÄSST GRÜßEN mit guten einwänden 
================================================================================
FLIGHT CONTROL – ARCHITEKTUR & ENTSCIDUNGEN (WARUM DAS SO GEMACHT WIRD)
================================================================================

GRUNDIDEE
---------
Die Flugregelung läuft in EINEM FreeRTOS-Task mit fester Grundfrequenz
(z. B. 500 Hz). Es gibt KEINE separaten Tasks für Attitude-, Rate- oder
Z-Regelung. Mehr Tasks erzeugen Jitter, Phasenverschiebung und schwer
debugbare Seiteneffekte. Regelung braucht Determinismus, nicht Parallelität.


ZEITSKALEN / KASKADIERUNG
------------------------
Die Regelung ist kaskadiert:

    Attitude  (langsam, z. B. 100 Hz)
        ↓  liefert Rate-Setpoints
    Rate      (schnell, z. B. 500 Hz)
        ↓
    Motor-Mixer

Der Rate-Controller läuft in JEDEM Loop-Durchlauf.
Der Attitude-Controller läuft seltener über einen Zeitakkumulator.
Beide teilen sich dieselbe Zeitbasis.

WICHTIG:
Zeit (dt) wird vom Control-Task vorgegeben und an die PIDs übergeben.
PID-Objekte messen KEINE Zeit selbst.


PID-DESIGN
----------
PID-Klassen sind zustandsbehaftete Rechenbausteine, keine Scheduler.
Sie bekommen:
- aktuellen Fehler oder Messwert
- ein explizites dt
und liefern einen Ausgang.

Keine:
- esp_timer_get_time() im PID
- versteckte Zeitmessung
- Nebenläufigkeit


DERIVATIVE-ANTEIL
-----------------
Der D-Term sollte bevorzugt auf dem Messwert basieren, nicht auf dem
Fehler. Das vermeidet Derivative-Kicks bei Setpoint-Sprüngen
(z. B. Stick-Input) und reduziert Rauschen.

Ein Lowpass auf dem D-Term ist Pflicht, idealerweise dt-abhängig
und nicht mit fixem Alpha.


INTEGRAL & ANTI-WINDUP
---------------------
Motoren sättigen IMMER.
Ohne Anti-Windup läuft das Integral weg und destabilisiert das System
nach der Sättigung.

Mindestens erforderlich:
- Output-Clamp
- Integral-Clamp
- Reset des Integrals bei Modewechsel (DISARM, FAILSAFE)


ACHSENSTRUKTUR
--------------
Pitch, Roll und Yaw sind strukturell identisch.
Daher:
- Eine Funktion pro Achse
- Je Achse: Attitude-PID + Rate-PID
- Keine verstreuten PID-Aufrufe im Task

Das reduziert Code-Duplikation und verhindert asymmetrische Bugs.


MOTOR-MIX & DESATURATION
-----------------------
Nach dem Motor-Mix muss geprüft werden, ob ein Motor clippt.
Wenn ja:
- zuerst Throttle reduzieren
- Attitude so gut wie möglich erhalten

Einfaches Clamping zerstört Drehmomente und führt zu Integrator-Fehlern.


Z-REGELUNG
----------
Vertikale Regelung koppelt stark in Attitude.
Rauschen oder Latenz erzeugen Throttle-Pumpen.

Empfohlen:
- Lowpass auf Z-Speed
- oder Feedforward
- konservative Gains


FREE RTOS – WICHTIGE KLARSTELLUNG
--------------------------------
FreeRTOS ist kein Regelungswerkzeug.
Mehr Tasks, höhere Prioritäten oder kürzere Delays machen die Regelung
NICHT besser.

Eine gute Regelung ist:
- zeitlich deterministisch
- explizit getaktet
- so seriell wie möglich


MERKSATZ
--------
Eine ruhige Zeitbasis fliegt besser als ein cleverer Scheduler.
================================================================================
*/
