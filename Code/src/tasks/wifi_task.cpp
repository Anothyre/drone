#include "tasks/wifi_task.h"
#include "tasks/fsm_task.h"
#include "tasks/wled_task.h"
#include <Arduino.h>
#include <WiFiUdp.h>
#include <WiFi.h>

static WiFiUDP udp;
static IPAddress last_remote_ip;
static uint16_t last_remote_port;
static uint32_t last_rx_time = 0;
static bool control_link_timed_out = false;

static void wifi_init();
static uint16_t crc16(const uint8_t *data, size_t len);

void TaskWiFi(void *pvParameters)
{
    wifi_init();
    last_rx_time = millis();

    control_packet_t pkt;

    for (;;)
    {
        int packetSize = udp.parsePacket();
        if (packetSize == sizeof(control_packet_t))
        {

            udp.read((uint8_t *)&pkt, sizeof(pkt));

            // Save sender (for telemetry)
            last_remote_ip = udp.remoteIP();
            last_remote_port = udp.remotePort();

            // Validate packet
            if (pkt.magic != 0x44524F4E)
                continue;

            uint16_t crc_rx = pkt.crc;
            pkt.crc = 0;
            if (crc16((uint8_t *)&pkt, sizeof(pkt)) != crc_rx) 
                continue;

            last_rx_time = millis();
            control_link_timed_out = false;

            // Push to FSM
            xQueueSend(fsm_command_queue, &pkt, 0);//just send fsm command not pkt
            


            xQueueSend(inputQueue, &pkt, 0);
            // Push to WLED for telemetry display
            xQueueSend(wled_command_queue, &pkt, 0);
        }

        // CONTROL LINK TIMEOUT
        const bool is_timed_out = (millis() - last_rx_time > CONTROL_TIMEOUT_MS);
        if (is_timed_out && !control_link_timed_out)
        {
            event_t evt = EV_EXCEPTION;
            xQueueSend(fsm_event_queue, &evt, 0);
            control_link_timed_out = true;
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
/**
 * CRC-16-IBM Calculation
 *  link: https://en.wikipedia.org/wiki/Cyclic_redundancy_check#Polynomial_representations
 */

static uint16_t crc16(const uint8_t *data, size_t len) // whispers words of wisdom
{
    uint16_t crc = 0xFFFF;
    while (len--)
    {
        crc ^= *data++;
        for (int i = 0; i < 8; i++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

static void wifi_init()
{
    Serial.println("Initializing WiFi...");
    delay(100);
    
    WiFi.mode(WIFI_AP);
    delay(100);
    
    bool ap_ok = WiFi.softAP("DRONE_FC", "drone123");//TODO: Might change password 
    delay(500);
    
    if (ap_ok) {
        Serial.print("WiFi AP started. IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("ERROR: Failed to start WiFi AP!");
    }
    
    if (udp.begin(CONTROL_PORT)) {
        Serial.printf("UDP listening on port %d\n", CONTROL_PORT);
    } else {
        Serial.println("ERROR: Failed to start UDP!");
    }
}
