#include "tasks/gps_task.h"
#include <Arduino.h>
#include <TinyGPS++.h>

TinyGPSPlus gps; // globaler Zugriff, pfui! - Dining Philosophers
HardwareSerial gpsSerial(GPS_UART);


#include "data_structures.h"

gps_data_t gps_data;


void TaskGPS(void *pvParameters)
{
    // Initialize GPS UART
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    uint32_t last_valid = 0;

    for (;;)
    {
        // Non-blocking serial read
        while (gpsSerial.available() > 0)
        {
            char c = gpsSerial.read();
            gps.encode(c);
        }

        // Process valid data (1–5Hz typical)
        if (gps.location.isUpdated())
        {
            // Validate before use
            if (gps.location.isValid() &&
                gps.satellites.value() >= 6 && // TODO: thik about magic numbers
                gps.hdop.hdop() < 2.0)
            {
                gps_data.lat = gps.location.lat();
                gps_data.lon = gps.location.lng();
                gps_data.alt = gps.altitude.meters();
                gps_data.speed = gps.speed.kmph();
                gps_data.course = gps.course.deg();
                gps_data.satellites = gps.satellites.value();
                gps_data.hdop = gps.hdop.hdop(); // trustworthyness
                gps_data.valid = true;
                gps_data.timestamp = xTaskGetTickCount();

                last_valid = xTaskGetTickCount();
            }
            else
            {
                // Invalidate old data if no fix for > 5 seconds
                if ((xTaskGetTickCount() - last_valid) > 5000)
                {
                    gps_data.valid = false;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz loop (GPS updates at 1–5Hz)
    }
}
/*
@ -1,56 +1,57 @@
#include "tasks/gps_task.h"
#include <Arduino.h>
#include <TinyGPS++.h>
    #include "tasks/gps_task.h"
    #include "data_structures.h"
    #include <Arduino.h>
    #include <TinyGPS++.h>

TinyGPSPlus gps; // globaler Zugriff, pfui! - Dining Philosophers
HardwareSerial gpsSerial(GPS_UART);
        TinyGPSPlus gps; // globaler Zugriff, pfui! - Dining Philosophers
    HardwareSerial gpsSerial(GPS_UART);

void TaskGPS(void *pvParameters)
{
    // Initialize GPS UART
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    uint32_t last_valid = 0;

    for (;;)
    void TaskGPS(void *pvParameters)
    {
        // Non-blocking serial read
        while (gpsSerial.available() > 0)
        {
            char c = gpsSerial.read();
            gps.encode(c);
        }
        // Initialize GPS UART
        gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

        uint32_t last_valid = 0;

        // Process valid data (1–5Hz typical)
        if (gps.location.isUpdated())
        for (;;)
        {
            // Validate before use
            if (gps.location.isValid() &&
                gps.satellites.value() >= 6 && // TODO: thik about magic numbers
                gps.hdop.hdop() < 2.0)
            // Non-blocking serial read
            while (gpsSerial.available() > 0)
            {
                gps_data.lat = gps.location.lat();
                gps_data.lon = gps.location.lng();
                gps_data.alt = gps.altitude.meters();
                gps_data.speed = gps.speed.kmph();
                gps_data.course = gps.course.deg();
                gps_data.satellites = gps.satellites.value();
                gps_data.hdop = gps.hdop.hdop(); // trustworthyness
                gps_data.valid = true;
                gps_data.timestamp = xTaskGetTickCount();

                last_valid = xTaskGetTickCount();
                char c = gpsSerial.read();
                gps.encode(c);
            }
            else

            // Process valid data (1–5Hz typical)
            if (gps.location.isUpdated())
            {
                // Invalidate old data if no fix for > 5 seconds
                if ((xTaskGetTickCount() - last_valid) > 5000)
                // Validate before use
                if (gps.location.isValid() &&
                    gps.satellites.value() >= 6 && // TODO: thik about magic numbers
                    gps.hdop.hdop() < 2.0)
                {
                    gps_data.lat = gps.location.lat();
                    gps_data.lon = gps.location.lng();
                    gps_data.alt = gps.altitude.meters();
                    gps_data.speed = gps.speed.kmph();
                    gps_data.course = gps.course.deg();
                    gps_data.satellites = gps.satellites.value();
                    gps_data.hdop = gps.hdop.hdop(); // trustworthyness
                    gps_data.valid = true;
                    gps_data.timestamp = xTaskGetTickCount();

                    last_valid = xTaskGetTickCount();
                }
                else
                {
                    gps_data.valid = false;
                    // Invalidate old data if no fix for > 5 seconds
                    if ((xTaskGetTickCount() - last_valid) > 5000)
                    {
                        gps_data.valid = false;
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz loop (GPS updates at 1–5Hz)
            vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz loop (GPS updates at 1–5Hz)
        }
    }
}
*/