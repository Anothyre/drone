#include "Arduino.h"
#include "tasks/adc_task.h"
#include "shared.h"

void TaskADC(void *pvParameters)
{
    // Init ADC
    analogReadResolution(ADC_RESOLUTION);
    analogSetPinAttenuation(ADC_CURRENT_PIN, ADC_GAIN);
    analogSetPinAttenuation(ADC_VOLTAGE_PIN, ADC_GAIN);

    // Init previous data (Assuming full charge and still motors)
    static float p_batteryCurrent = INITIAL_BATTERY_CURRENT;
    static float p_batteryVoltage = INITIAL_BATTERY_VOLTAGE;

    for (;;)
    {
        ADCSample_t sample;

        // Read raw Data
        uint16_t rawCurrent     = analogRead(ADC_CURRENT_PIN);
        uint16_t rawVoltage     = analogRead(ADC_VOLTAGE_PIN);

        // Calculate actual current
        float currentVoltage    = ((float)rawCurrent / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
        float shuntVoltage      = currentVoltage / SHUNT_AMP_RATIO;
        float batteryCurrent    = shuntVoltage / SHUNT_RESISTANCE;

        // Calculate actual voltage
        float busVoltage        = ((float)rawVoltage / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
        float batteryVoltage    = busVoltage * VOLTAGE_DIVIDER_RATIO;

        // Average with last value for smoothing
        batteryCurrent          = (batteryCurrent + p_batteryCurrent) / 2.0f;
        p_batteryCurrent        = batteryCurrent;

        batteryVoltage          = (batteryVoltage + p_batteryVoltage) / 2.0f;
        p_batteryVoltage        = batteryVoltage;

        // Fill struct
        sample.batteryCurrent = batteryCurrent;
        sample.batteryVoltage = batteryVoltage;

        // Send to queue (non-blocking or short timeout recommended)
        xQueueSend(ADCQueue, &sample, 0);

        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz
    }
}
