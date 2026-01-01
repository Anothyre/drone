#include "tasks/wifi_task.h"
#include "tasks/fsm_task.h"
#include <Arduino.h>
#include <WiFiUdp.h>
#include <WiFi.h>

static WiFiUDP udp;
static IPAddress last_remote_ip;
static uint16_t last_remote_port;
static uint32_t last_rx_time = 0;

void TaskWiFi(void *pvParameters)
{
    wifi_init();

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

            // Push to FSM
            xQueueSend(fsm_command_queue, &pkt, 0);
        }

        // CONTROL LINK TIMEOUT
        if (millis() - last_rx_time > CONTROL_TIMEOUT_MS)
        {
            event_t evt = EV_EXCEPTION;
            xQueueSend(fsm_event_queue, &evt, 0);
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
    WiFi.mode(WIFI_AP);
    WiFi.softAP("DRONE_FC", "drone123"); // TODO: maybe change passwrd, maybe not. Maybe also set as env variable or smth idk.
    // soft means no internet, just local network <- BS! SoftAP is an abbreviated term for "software enabled access point". Such access points utilize software to enable a computer which hasn't been specifically made to be a router into a wireless access point. It is often used interchangeably with the term "virtual router".
    udp.begin(CONTROL_PORT); // control port means the port we listen to for incoming control packets
    Serial.println("WiFi AP started. Waiting for control packets...");
}
