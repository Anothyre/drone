#include "tasks/wled_task.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// --- WLED HARDWARE ---
#define LED_PIN 12
#define NUM_LEDS 6
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

static uint8_t axisToIntensity(float value) {
    float v = fabsf(value);
    if (v > 1.0f) v = 1.0f;
    return (uint8_t)(v * 255.0f);
}

static void renderLeds(const control_packet_t* pkt) {
    if (!pkt) {
        for (int i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, 0);
        }
        strip.show();
        return;
    }

    // LED0: Link status (green)
    strip.setPixelColor(0, strip.Color(0, 30, 0));

    // LED1: X axis (left/right): red for negative, green for positive
    uint8_t xMag = axisToIntensity(pkt->x);
    strip.setPixelColor(1, pkt->x >= 0 ? strip.Color(0, xMag, 0) : strip.Color(xMag, 0, 0));

    // LED2: Y axis (forward/back): cyan for positive, magenta for negative
    uint8_t yMag = axisToIntensity(pkt->y);
    strip.setPixelColor(2, pkt->y >= 0 ? strip.Color(0, yMag, yMag) : strip.Color(yMag, 0, yMag));

    // LED3: Z axis (throttle): blue intensity 0..1
    uint8_t zMag = axisToIntensity(pkt->z);
    strip.setPixelColor(3, strip.Color(0, 0, zMag));

    // LED4: Yaw axis: yellow for positive, purple for negative
    uint8_t yawMag = axisToIntensity(pkt->yaw);
    strip.setPixelColor(4, pkt->yaw >= 0 ? strip.Color(yawMag, yawMag, 0) : strip.Color(yawMag, 0, yawMag));

    // LED5: Event status
    if (pkt->mode == 4) {          // ARM
        strip.setPixelColor(5, strip.Color(255, 128, 0));
    } else if (pkt->mode == 5) {   // TAKEOFF
        strip.setPixelColor(5, strip.Color(255, 255, 255));
    } else if (pkt->mode == 14) {  // LAND
        strip.setPixelColor(5, strip.Color(255, 0, 255));
    } else {
        strip.setPixelColor(5, strip.Color(0, 20, 0));
    }

    strip.show();
}

void TaskWLED(void *pvParameters) {
    strip.begin();
    strip.setBrightness(80);
    strip.fill(0);
    strip.show();

    Serial.println("WLED Task started");

    control_packet_t pkt;

    for (;;) {
        if (xQueueReceive(wled_command_queue, &pkt, pdMS_TO_TICKS(100)) == pdPASS) {//to fast i guess
            renderLeds(&pkt);
        } else {
            // Timeout - show all off
            strip.fill(100);
            strip.show();
        }
    }
}
