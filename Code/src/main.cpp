#include <Arduino.h>
#include "tasks.h"
#include "data_structures.h"
#include "core_config.h"
#include "shared.h"
#include <tasks/fsm_task.h>
#include <queue.h>
#include <tasks/wifi_task.h>

// PWM Pins und Konstanten
#define MOTOR_Start 1
#define MOTOR_End 4
#define LED_Start 8
#define LED_End 14
#define BUZZER 21 

// Einheitliche Benennung
const int freq = 2000;       
const int ledc_channel = 2;  // Kanal 2 nutzen (0 und 1 meiden beim S3)
const int resolution = 8;    

void testBuzzer() {
  // Pin sauber initialisieren
  pinMode(BUZZER, INPUT); 

  // LEDC Konfiguration für ESP32 Core 2.0.17
  ledcSetup(ledc_channel, freq, resolution);
  ledcAttachPin(BUZZER, ledc_channel);
  
  // 50% Duty Cycle (Rechteckwelle erzeugen)
  ledcWrite(ledc_channel, 128); 
  
  // Da hier noch keine FreeRTOS-Tasks laufen, blockiert ets_delay_us oder ein sauberes delay nicht die Architektur
  delay(4000); 
  
  // Stoppen
  ledcWrite(ledc_channel, 0);
  ledcDetachPin(BUZZER);
  
  Serial.println("[HARDWARE] Buzzer Test finished.");
}

void setup()
{
  Serial.begin(115200);
  
  // USB-CDC Startup-Verzögerung
  for(int i = 1; i <= 4; i++){ 
    delay(1000); 
    Serial.printf("--- Alive for %d sec---\n", i); 
  }
  
  Serial.println("--- Alive --- with version: " + String(ESP_ARDUINO_VERSION_MAJOR) + "." + String(ESP_ARDUINO_VERSION_MINOR) + "." + String(ESP_ARDUINO_VERSION_PATCH));
  Serial.flush();

  // HARDWARE-TESTS ZUERST (Bevor der Scheduler dazwischenfunkt!)
  Serial.println("[SETUP] hardware test starting...");
  Serial.flush();

  for(int pin = LED_Start; pin <= LED_End; pin++){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  // Buzzer testen, solange die CPU exklusiv uns gehört
  testBuzzer();

  delay(2000); 
  for(int pin = LED_Start+1; pin <= LED_End; pin++){
    digitalWrite(pin, LOW);
  }
  Serial.println("[SETUP] hardware test done");
  Serial.flush();

  // JETZT ERST DIE TASKS UND QUEUES STARTEN
  Serial.println("[SETUP] initQueues starting...");
  Serial.flush();
  initQueues();
  Serial.println("[SETUP] initQueues done");
  Serial.flush();
  
  Serial.println("[SETUP] create_tasks starting...");
  Serial.flush();
  create_tasks();
  Serial.println("[SETUP] create_tasks done");
  Serial.flush();
  
  Serial.println("Setup complete.");
  Serial.flush();
}

void loop()
{
  // Schlafen legen, da FreeRTOS-Tasks die Arbeit machen
  vTaskDelay(pdMS_TO_TICKS(1000));
}