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
// Konfiguration für Buzzer (2 kHz, 8-Bit)
const int buzzer_freq = 2000;       
const int buzzer_channel = 0;  
const int buzzer_res = 8;    

// Konfiguration für ESC-Motoren (50 Hz, 14-Bit für präzise µs)
const int esc_freq = 50;
const int esc_res = 14;

// Signal-Grenzwerte bei 14-Bit (2^14 = 16384 Ticks bei 20ms Periodendauer)
const int ESC_THROTTLE_MIN = 819;  // entspricht ~1.0 ms (Nullgas / Arming)
const int ESC_THROTTLE_TEST = 900; // Minimales Gas zum Testen (~1.1 ms)

void testBuzzer() {
  pinMode(BUZZER, INPUT); 
  ledcSetup(buzzer_channel, buzzer_freq, buzzer_res);
  ledcAttachPin(BUZZER, buzzer_channel);
  
  ledcWrite(buzzer_channel, 128); 
  delay(1000); 
  
  ledcWrite(buzzer_channel, 0);
  ledcDetachPin(BUZZER);
  Serial.println("[HARDWARE] Buzzer Test finished.");
}

void testMotors() {
  Serial.println("[HARDWARE] Initialisiere ESCs (Sende Nullgas)...");
  
  // 1. Alle Motoren auf eigenen Kanälen mit 50Hz initialisieren und auf Nullgas setzen
  for(int pin = MOTOR_Start; pin <= MOTOR_End; pin++){
    int current_channel = pin + 1; // Kanal 2, 3, 4, 5 nutzen (Kanal 0/1 meiden)
    
    pinMode(pin, INPUT); // Aus digitalem Zustand lösen
    ledcSetup(current_channel, esc_freq, esc_res);
    ledcAttachPin(pin, current_channel);
    
    // Sofort 1.0ms Puls senden, damit der ESC beim Booten nicht in den Error-Mode geht
    ledcWrite(current_channel, ESC_THROTTLE_MIN); 
  }

  // 2. Halte das Nullgas-Signal für 4 Sekunden, damit die ESCs booten und "Scharf" schalten
  delay(4000); 
  Serial.println("[HARDWARE] ESCs armed. Starte kurzen Drehtest...");

  // 3. Signal leicht anheben, um zu sehen, ob die Motoren reagieren
  for(int pin = MOTOR_Start; pin <= MOTOR_End; pin++){
    int current_channel = pin + 1;
    ledcWrite(current_channel, ESC_THROTTLE_TEST); 
  }

  // Testlauf für 3 Sekunden
  delay(3000);

  // 4. Alle Motoren wieder stoppen und Peripherie freigeben
  for(int pin = MOTOR_Start; pin <= MOTOR_End; pin++){
    int current_channel = pin + 1;
    ledcWrite(current_channel, ESC_THROTTLE_MIN); 
    ledcDetachPin(pin);
  }
  
  Serial.println("[HARDWARE] Motor Test finished.");
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

  Serial.println("[SETUP] hardware test starting...");
  Serial.flush();

  for(int pin = LED_Start; pin <= LED_End; pin++){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  testBuzzer();
  testMotors();

  delay(2000); 
  for(int pin = LED_Start+1; pin <= LED_End; pin++){
    digitalWrite(pin, LOW);
  }
  Serial.println("[SETUP] hardware test done");
  Serial.flush();

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


  //test pwm
}

void loop()
{
  // Schlafen legen, da FreeRTOS-Tasks die Arbeit machen
  vTaskDelay(pdMS_TO_TICKS(1000));
}