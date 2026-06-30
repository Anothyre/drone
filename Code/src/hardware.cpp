#include "hardware.h"
#include "core_config.h"
#include <Arduino.h>

// Pin mapping for the requested LEDs
#define LED_ALIVE 8
#define LED_ARMED 9
#define LED_WARNING 10
#define LED_GPS 11
#define LED_BATTERY_LOW 12
#define LED_WIFI 13
#define LED_CAMERA 14

#define BUZZER_PIN 21
#define MOTOR_START 1
#define MOTOR_END 4

#define BUZZER_FREQ 2000
#define BUZZER_CHANNEL 0
#define BUZZER_RES 8

#define ESC_FREQ 50
#define ESC_RES 14
#define ESC_THROTTLE_MIN 819
#define ESC_THROTTLE_IDLE 850
#define ESC_THROTTLE_ARM 900

static const uint8_t led_pins[] = {
    LED_ALIVE,
    LED_ARMED,
    LED_WARNING,
    LED_GPS,
    LED_BATTERY_LOW,
    LED_WIFI,
    LED_CAMERA
};

static bool hardware_led_states[7] = {false, false, false, false, false, false, false};

static void hardware_setup_leds(void)
{
    for (size_t i = 0; i < sizeof(led_pins) / sizeof(led_pins[0]); ++i) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], LOW);
    }
}

static void hardware_setup_buzzer(void)
{
    pinMode(BUZZER_PIN, OUTPUT);
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RES);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0);
}

static void hardware_setup_motors(void)
{
    for (int pin = MOTOR_START; pin <= MOTOR_END; ++pin) {
        int current_channel = pin + 1;
        pinMode(pin, OUTPUT);
        ledcSetup(current_channel, ESC_FREQ, ESC_RES);
        ledcAttachPin(pin, current_channel);
        ledcWrite(current_channel, ESC_THROTTLE_MIN);
    }
}

void hardware_init(void)
{
    hardware_setup_leds();
    hardware_setup_buzzer();
    hardware_setup_motors();
}

void hardware_run_boot_sequence(void)
{
    hardware_set_system_status(true, false, false, false, false, false, false);
    hardware_beep(1, 120, 80);
    delay(200);

    hardware_set_system_status(true, true, false, false, false, false, false);
    delay(200);

    hardware_set_system_status(true, false, true, false, false, false, false);
    delay(200);

    hardware_set_system_status(true, false, false, true, false, false, false);
    delay(200);

    hardware_set_system_status(true, false, false, false, true, false, false);
    delay(200);

    hardware_set_system_status(true, false, false, false, false, true, false);
    delay(200);

    hardware_set_system_status(true, false, false, false, false, false, true);
    delay(200);

    hardware_set_system_status(true, false, false, false, false, false, false);
    hardware_set_motors_idle();
}

void hardware_set_led(uint8_t led_index, bool on)
{
    if (led_index >= sizeof(led_pins) / sizeof(led_pins[0])) {
        return;
    }

    hardware_led_states[led_index] = on;
    digitalWrite(led_pins[led_index], on ? HIGH : LOW);
}

void hardware_set_alive(bool on)
{
    hardware_set_led(0, on);
}

void hardware_set_armed(bool on)
{
    hardware_set_led(1, on);
}

void hardware_set_warning(bool on)
{
    hardware_set_led(2, on);
}

void hardware_set_gps(bool on)
{
    hardware_set_led(3, on);
}

void hardware_set_battery_low(bool on)
{
    hardware_set_led(4, on);
}

void hardware_set_wifi(bool on)
{
    hardware_set_led(5, on);
}

void hardware_set_camera(bool on)
{
    hardware_set_led(6, on);
}

void hardware_set_system_status(bool alive, bool armed, bool warning, bool gps, bool battery_low, bool wifi, bool camera)
{
    hardware_set_alive(alive);
    hardware_set_armed(armed);
    hardware_set_warning(warning);
    hardware_set_gps(gps);
    hardware_set_battery_low(battery_low);
    hardware_set_wifi(wifi);
    hardware_set_camera(camera);
}

void hardware_beep(uint8_t count, uint32_t on_ms, uint32_t off_ms)
{
    for (uint8_t i = 0; i < count; ++i) {
        ledcWrite(BUZZER_CHANNEL, 128);
        delay(on_ms);
        ledcWrite(BUZZER_CHANNEL, 0);
        if (i + 1 < count) {
            delay(off_ms);
        }
    }
}

void hardware_set_motor_throttle(uint8_t motor_index, uint16_t pulse_ticks)
{
    if (motor_index >= 4) {
        return;
    }

    int pin = MOTOR_START + motor_index;
    int current_channel = pin + 1;
    ledcWrite(current_channel, pulse_ticks);
}

void hardware_set_motors_idle(void)
{
    for (uint8_t i = 0; i < 4; ++i) {
        hardware_set_motor_throttle(i, ESC_THROTTLE_IDLE);
    }
}

void hardware_set_motors_enabled(bool enabled)
{
    for (uint8_t i = 0; i < 4; ++i) {
        hardware_set_motor_throttle(i, enabled ? ESC_THROTTLE_IDLE : ESC_THROTTLE_MIN);
    }
}
