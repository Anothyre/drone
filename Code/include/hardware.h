#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

void hardware_init(void);
void hardware_run_boot_sequence(void);

void hardware_set_led(uint8_t led_index, bool on);
void hardware_set_alive(bool on);
void hardware_set_armed(bool on);
void hardware_set_warning(bool on);
void hardware_set_gps(bool on);
void hardware_set_battery_low(bool on);
void hardware_set_wifi(bool on);
void hardware_set_camera(bool on);
void hardware_set_system_status(bool alive, bool armed, bool warning, bool gps, bool battery_low, bool wifi, bool camera);

void hardware_beep(uint8_t count, uint32_t on_ms, uint32_t off_ms);
void hardware_set_motor_throttle(uint8_t motor_index, uint16_t pulse_ticks);
void hardware_set_motors_idle(void);
void hardware_set_motors_enabled(bool enabled);

#endif
