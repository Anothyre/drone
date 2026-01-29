#include <Arduino.h>
#include "tasks.h"
#include "data_structures.h"
#include "core_config.h"
#include "shared.h"
#include <tasks/fsm_task.h>
#include <queue.h>
#include <tasks/wifi_task.h>
// #include <ESP32Servo.h>

#define BUZZER 21 // Pin für den Summer
#define LED_Start 8// Pin für die eingebauten LEDs (ESP32)
#define LED_End 14
#define MOTOR1 1
#define MOTOR2 2
#define MOTOR3 3
#define MOTOR4 4



#include <Arduino.h>
#include "tasks.h"
#include "data_structures.h"
#include "core_config.h"
#include "shared.h"
#include <tasks/fsm_task.h>
#include <queue.h>
#include <tasks/wifi_task.h>

// --- Hardware ---
#define BUZZER     21
#define LED_Start   8
#define LED_End    14

#define MOTOR1     1  

// --- LEDC / PWM ---
constexpr int MOTOR_PIN   = MOTOR1;
constexpr int LEDC_CH     = 2;     // PWM-Kanal
constexpr uint32_t PWM_FREQ = 50;  // 50 Hz = Servo/ESC
constexpr uint8_t PWM_RES  = 12;   // 12 Bit → 0..4095


// Maximaler Duty-Wert bei 12 Bit
constexpr uint32_t MAX_DUTY = (1UL << PWM_RES) - 1;



void AliveTask(void* pvParameters) {
  // ledcAttachPin(MOTOR_PIN, LEDC_CH);//koruppt - wenn da kein ton 
        tone(BUZZER, 440*2);

        vTaskDelay(pdMS_TO_TICKS(2000));         




   

    for(int pin = LED_Start; pin <= LED_End; pin++){
        pinMode(pin, OUTPUT);
    }
    while (true) {
        
   Serial.println("[AliveTask] Motor ramp test");

    for (uint32_t duty = 0; duty <= MAX_DUTY; duty += MAX_DUTY / 100) {
        ledcWrite(LEDC_CH, duty);              // PWM setzen
        Serial.printf("Duty: %lu / %lu\n", duty, MAX_DUTY);
        vTaskDelay(pdMS_TO_TICKS(5));         // langsam hochfahren
    }

    // Motor wieder aus
    ledcWrite(LEDC_CH, 0);
    Serial.println("[AliveTask] Motor off");


      for(int pin = LED_Start; pin <= LED_End; pin++){
        digitalWrite(pin, HIGH);
      }
      int tM = 3;
      for(int i = 0; i<3;i++){
      tone(BUZZER, 440*2);
      vTaskDelay(pdMS_TO_TICKS(100*tM));
      noTone(BUZZER);
      vTaskDelay(pdMS_TO_TICKS(50));
      }
      tone(BUZZER, 370*2);
      vTaskDelay(pdMS_TO_TICKS(100*tM*3));
      noTone(BUZZER);

      vTaskDelay(pdMS_TO_TICKS(50));
      noTone(BUZZER);
      vTaskDelay(pdMS_TO_TICKS(200*tM));

       for(int i = 0; i<3;i++){
      tone(BUZZER, 392*2);
      vTaskDelay(pdMS_TO_TICKS(100*tM));
      noTone(BUZZER);
      vTaskDelay(pdMS_TO_TICKS(50));
      }
      tone(BUZZER, 330*2);
      vTaskDelay(pdMS_TO_TICKS(100*tM*3));
      noTone(BUZZER);
      vTaskDelay(pdMS_TO_TICKS(50));

      for(int pin = LED_Start; pin <= LED_End; pin++){
        digitalWrite(pin, LOW);
      }
        

        // Längere Pause nach der Phrase, damit es nicht dauernd repeatet
        vTaskDelay(10000); 
    }
}






void setup()
{

  Serial.begin(115200);
  delay(2000);
  Serial.println("--- Alive ---");//TODO text
   xTaskCreate(
        AliveTask,       // Task-Funktion
        "AliveTask",     // Name
        2048,            // Stack-Größe
        NULL,            // Parameter
        1,               // Priorität
        NULL             // Task-Handle
    );


  //create_tasks();//TODO auskommentiert zum testen
  initQueues(); 

  Serial.println("Setup complete.");
}

void loop()
{
  //Kas - brach ma ned
  vTaskDelay(1000);
}


/*
-- Tasks --
alles was parrallel läuft oder eine andere Verabeitungsgeschwindichkeit braucht muss in einen eigenen Task




Wifi Control (In eigenem Kern!!) - nein eigentlich Regler eigener Kern oder so 

Statemachine
- Regler Regler(evtl.)
- -Regler (vlt. mit SM gemeinsam)

Sensoren
- Atitude? Wo bin ich? Wie bin ich? Bin ich? Was ist die menschliche Bedingung? Wer bin ich? All the world's a stage, And all the Drones and Planes merely players;


Suggested tasks (names, priority, frequency, stack, IPC)

(
  IMU IRQ handler — ISR (hardware)
  Priority: N/A (ISR)
  Role: respond to DRDY (data ready) interrupt, DMA complete; copy raw sample into a lock-free ring buffer / double buffer, give a task notification to IMU_Task.
  Notes: keep ISR minimal; no heavy FP math.
)

Attitude task (IMU processing + AHRS) — IMU_Task
Priority: 6 (highest)
Frequency: 250–1000 Hz (choose based on gyro sample rate & CPU — 400–500Hz is common)
Stack (estimate): 6–12 KB (depends on filter implementation and FPU use)
Work: pull raw IMU samples (from ISR buffer), run bias compensation, calibration, run AHRS (Mahony / Madgwick or small EKF), publish attitude to shared_state (double buffer + atomic swap), notify Control_Task.
IPC: task notification to trigger control; write to attitude_queue or atomic struct.

Control task (inner + outer loop manager) — Control_Task
Priority: 5
Frequency: inner loop 250–500Hz (attitude rate); outer loop (velocity/altitude) typically 50–200Hz
Stack: 8–16 KB
Work: read latest attitude (atomic), run inner PID (rate/attitude), compute motor outputs, perform mixing, send commands to motor output driver (PWM/RMT). Outer loop controllers can run at lower sub-rate.
IPC: reads from attitude double buffer; may receive setpoints via setpoint_queue from comms.

Sensor tasks (GPS, Baro, Mag if separate, ADC current/voltage) — GPS_Task, Baro_Task, ADC_Task
Priority: 4 (GPS lower priority than control)
Frequency: GPS 1–10Hz; baro 25–50Hz; ADC 50–200Hz
Stack: GPS 4–8KB, Baro 4KB, ADC 4KB
Work: decode NMEA or UBX (GPS), time stamp and publish via queue; baro filter & publish; ADC read & publish.
IPC: push sensor_queues (timestamped data)

State estimator / Position EKF — State_EKF_Task
Priority: 3
Frequency: 50–200Hz (depending on desired position update)
Stack: 8–12 KB
Work: fuse IMU (integrated), baro, GPS, magnetometer into position/velocity state. Use IMU preintegration between EKF updates. Publish nav state to shared_state.
IPC: consumes IMU preintegrated deltas (from IMU_Task) and slower sensor queues.

Telemetry / Command task — WiFi_Task
Priority: 2
Frequency: event-driven / e.g. telemetry 10–100Hz
Stack: 8–12 KB (TLS or serialization increases stack)
Work: manage Wi-Fi connection, send telemetry frames, receive command packets, validate and push commands to setpoint_queue. Use non-blocking sockets; prefer UDP for low latency telemetry and a TCP (or UDP+ACK) control channel if commands require reliability.
IPC: setpoint_queue, telemetry_queue
Logging / SD / Telemetry storage — Log_Task
Priority: 1 (lowest)
Frequency: lower (burst as needed)
Stack: 6–10 KB
Work: write logs to SD/flash, handle large payloads; non-blocking, truncates when full.

Housekeeping / CLI / OTA — Housekeeping_Task
Priority: 1–2
Frequency: event driven
Work: low priority maintenance (OTA updates, CLI commands, health checks).
*/
