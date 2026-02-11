#ifndef ADC_TASK_H
#define ADC_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void TaskADC(void *pvParameters);

#define ADC_VOLTAGE_PIN         5
#define ADC_CURRENT_PIN         6
#define ADC_GAIN                ADC_11db
#define ADC_RESOLUTION          12
#define ADC_MAX_VALUE           4095.0f     // 2^12
#define ADC_REF_VOLTAGE         3.3f

#define VOLTAGE_DIVIDER_RATIO   5.3         // (34kOhm + 10kOhm) / 10kOhm

#define SHUNT_AMP_RATIO         50          // 50V/V
#define SHUNT_RESISTANCE        0.5e-3      // 0.5mOhm
#define INITIAL_CURRENT         0           // 0A
#define INITIAL_BATTERY_VOLTAGE         16.8f       // 4S LIPO full voltage

#endif // ADC_TASK_H