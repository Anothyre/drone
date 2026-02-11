#include "Arduino.h"
#include "tasks/adc_task.h"

void TaskADC(void *pvParameters)
{

    // Init ADC
    analogReadResolution(ADC_RESOLUTION);                   // 0–4095
    analogSetPinAttenuation(ADC_CURRENT_PIN, ADC_GAIN);     // up to ~3.3V
    analogSetPinAttenuation(ADC_VOLTAGE_PIN, ADC_GAIN);

    for (;;)
    {
        // Init previous data (Assuming full charge and stil Motors)
        static float p_current = INITIAL_CURRENT;
        static float p_batteryVoltage = INITIAL_BATTERY_VOLTAGE;
        
        // Read raw Data
        uint16_t rawCurrent     = analogRead(ADC_CURRENT_PIN);
        uint16_t rawVoltage     = analogRead(ADC_VOLTAGE_PIN);

        // Calculate actual current
        float currentVoltage    = (rawCurrent / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
        float shuntVoltage      = currentVoltage / SHUNT_AMP_RATIO;
        float current           = shuntVoltage / SHUNT_RESISTANCE;

        // Calculate actual voltage
        float busVoltage        = (rawVoltage / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
        float batteryVoltage    = busVoltage * VOLTAGE_DIVIDER_RATIO;

        // Average with last value for smoothing
        current                 = (current + p_current) / 2.0f;
        p_current               = current;
        batteryVoltage          = (batteryVoltage + p_batteryVoltage) / 2.0f;
        p_batteryVoltage        = batteryVoltage;

        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz
    }
}
