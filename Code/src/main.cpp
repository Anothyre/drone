

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// --- WLED HARDWARE ---
#define LED_PIN 12
#define NUM_LEDS 6
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- NETZWERK SETUP ---
const char* ssid = "DRONE_FC";
const char* password = "drone123";
#define CONTROL_PORT 14550 // Muss exakt ESP_PORT im Python-Code entsprechen

WiFiUDP udp;

// --- PACKET STRUKTUR ---
// WICHTIG: __attribute__((packed)) verhindert, dass C++ leere Bytes (Padding) einfügt.
// Das muss exakt dem Python struct.pack("<I H I f f f f B", ...) entsprechen.
// Gesamtgröße: exakt 29 Bytes.
struct __attribute__((packed)) control_packet_t {
    uint32_t magic;         // I (4 Byte)
    uint16_t seq;           // H (2 Byte)
    uint32_t timestamp_ms;  // I (4 Byte)
    float x;                // f (4 Byte)
    float y;                // f (4 Byte)
    float z;                // f (4 Byte)
    float yaw;              // f (4 Byte)
    uint8_t mode;           // B (1 Byte)
    uint16_t crc;           // H (2 Byte)
};

static uint8_t axisToIntensity(float value) {
    float v = fabsf(value);
    if (v > 1.0f) v = 1.0f;
    return (uint8_t)(v * 255.0f);
}

// LED map (0..5):
// 0 link status, 1 x, 2 y, 3 z, 4 yaw, 5 event
static void renderTelemetryLeds(bool link_ok, const control_packet_t* pkt) {
    // LED0: Link status
    strip.setPixelColor(0, link_ok ? strip.Color(0, 30, 0) : strip.Color(30, 0, 0));

    if (!pkt) {
        for (int i = 1; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, 0);
        }
        strip.show();
        return;
    }

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

static uint16_t crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    while (len--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    // LEDs initialisieren
    strip.begin();
    strip.setBrightness(80);
    strip.fill(0);
    strip.show();

    // Access Point starten
    Serial.println("Starte WiFi AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    // UDP Server starten
    udp.begin(CONTROL_PORT);
    Serial.printf("AP bereit. IP: %s | UDP Port: %d\n", WiFi.softAPIP().toString().c_str(), CONTROL_PORT);

    // Link up, aber noch keine Daten
    renderTelemetryLeds(false, nullptr);
}

uint32_t last_rx_time = 0;

void loop() {
    int packetSize = udp.parsePacket();
    
    if (packetSize == sizeof(control_packet_t)) {
        control_packet_t pkt;
        udp.read((uint8_t *)&pkt, sizeof(pkt));

        if (pkt.magic == 0x44524F4E) {
            uint16_t crc_rx = pkt.crc;
            pkt.crc = 0; // Für CRC Check auf 0 setzen, genau wie in Python!
            
            if (crc16((uint8_t *)&pkt, sizeof(pkt)) == crc_rx) {
                last_rx_time = millis();

                // Serielle Ausgabe der Joystick-Daten
                Serial.printf("Seq: %5d | X: %+.2f Y: %+.2f Z: %+.2f Yaw: %+.2f | Event: %d\n", 
                              pkt.seq, pkt.x, pkt.y, pkt.z, pkt.yaw, pkt.mode);

                renderTelemetryLeds(true, &pkt);
            } else {
                Serial.println("CRC Fehler! Paket verworfen.");
            }
        }
    }

    // Timeout-Check (z.B. nach 500ms ohne Paket)
    if (millis() - last_rx_time > 500 && last_rx_time != 0) {
        renderTelemetryLeds(false, nullptr);
        last_rx_time = 0; // Reset, damit es nur einmal rot wird
    }

    delay(1); // Wichtig, damit der Watchdog den Core nicht abstürzen lässt
}


// #include <Arduino.h>
// #include "tasks.h"
// #include "data_structures.h"
// #include "core_config.h"
// #include "shared.h"
// #include <tasks/fsm_task.h>
// #include <queue.h>
// #include <tasks/wifi_task.h>



// void setup()
// {
//   Serial.begin(115200);
//   delay(2000);
//   Serial.println("--- Alive ---");
//   initQueues(); 
//   create_tasks();
//   Serial.println("Setup complete.");
// }

// void loop()
// {
//   //Kas - brach ma ned
//   vTaskDelay(1000);}



/*
-- Tasks --
alles was parrallel läuft oder eine andere Verabeitungsgeschwindichkeit braucht muss in einen eigenen Task




Wifi Control (In eigenem Kern!!) - nein eigentlich Regler eigener Kern oder so 

Statemachine
- Regler Regler(evtl.)
- -Regler (vlt. mit SM gemeinsam)

Sensoren
- Atitude? Wo bin ich? Wie bin ich? Bin ich? Was ist die menschliche Bedingung? Wer bin ich? All the world's a stage, And all the Drones and Planes merely players;


Suggested tasks (names, priority, frequency, stack, IPC)

(
  IMU IRQ handler — ISR (hardware)
  Priority: N/A (ISR)
  Role: respond to DRDY (data ready) interrupt, DMA complete; copy raw sample into a lock-free ring buffer / double buffer, give a task notification to IMU_Task.
  Notes: keep ISR minimal; no heavy FP math.
)

Attitude task (IMU processing + AHRS) — IMU_Task
Priority: 6 (highest)
Frequency: 250–1000 Hz (choose based on gyro sample rate & CPU — 400–500Hz is common)
Stack (estimate): 6–12 KB (depends on filter implementation and FPU use)
Work: pull raw IMU samples (from ISR buffer), run bias compensation, calibration, run AHRS (Mahony / Madgwick or small EKF), publish attitude to shared_state (double buffer + atomic swap), notify Control_Task.
IPC: task notification to trigger control; write to attitude_queue or atomic struct.

Control task (inner + outer loop manager) — Control_Task
Priority: 5
Frequency: inner loop 250–500Hz (attitude rate); outer loop (velocity/altitude) typically 50–200Hz
Stack: 8–16 KB
Work: read latest attitude (atomic), run inner PID (rate/attitude), compute motor outputs, perform mixing, send commands to motor output driver (PWM/RMT). Outer loop controllers can run at lower sub-rate.
IPC: reads from attitude double buffer; may receive setpoints via setpoint_queue from comms.

Sensor tasks (GPS, Baro, Mag if separate, ADC current/voltage) — GPS_Task, Baro_Task, ADC_Task
Priority: 4 (GPS lower priority than control)
Frequency: GPS 1–10Hz; baro 25–50Hz; ADC 50–200Hz
Stack: GPS 4–8KB, Baro 4KB, ADC 4KB
Work: decode NMEA or UBX (GPS), time stamp and publish via queue; baro filter & publish; ADC read & publish.
IPC: push sensor_queues (timestamped data)

State estimator / Position EKF — State_EKF_Task
Priority: 3
Frequency: 50–200Hz (depending on desired position update)
Stack: 8–12 KB
Work: fuse IMU (integrated), baro, GPS, magnetometer into position/velocity state. Use IMU preintegration between EKF updates. Publish nav state to shared_state.
IPC: consumes IMU preintegrated deltas (from IMU_Task) and slower sensor queues.

Telemetry / Command task — WiFi_Task
Priority: 2
Frequency: event-driven / e.g. telemetry 10–100Hz
Stack: 8–12 KB (TLS or serialization increases stack)
Work: manage Wi-Fi connection, send telemetry frames, receive command packets, validate and push commands to setpoint_queue. Use non-blocking sockets; prefer UDP for low latency telemetry and a TCP (or UDP+ACK) control channel if commands require reliability.
IPC: setpoint_queue, telemetry_queue
Logging / SD / Telemetry storage — Log_Task
Priority: 1 (lowest)
Frequency: lower (burst as needed)
Stack: 6–10 KB
Work: write logs to SD/flash, handle large payloads; non-blocking, truncates when full.

Housekeeping / CLI / OTA — Housekeeping_Task
Priority: 1–2
Frequency: event driven
Work: low priority maintenance (OTA updates, CLI commands, health checks).
*/
