#include <Arduino.h>
#include "tasks.h"
#include "data_structures.h"
#include "core_config.h"
#include "shared.h"
#include "hardware.h"
#include <tasks/fsm_task.h>
#include <queue.h>
#include <tasks/wifi_task.h>

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

  Serial.println("[SETUP] hardware init starting...");
  Serial.flush();

  hardware_init();
  hardware_run_boot_sequence();

  Serial.println("[SETUP] hardware init done");
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