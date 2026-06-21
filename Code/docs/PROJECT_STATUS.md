# Projektstatus / Implementierungsübersicht

## Gesamtübersicht
Dieses Projekt ist ein ESP32-basierter Drohnen-Flugcontroller unter PlatformIO. Die Architektur ist in mehrere FreeRTOS-Tasks aufgeteilt und nutzt Queues für den Datenaustausch.

Die wichtigsten Bereiche sind:
- Sensorerfassung: IMU, Barometer, GPS, ADC
- Zustandsschätzung / EKF (noch placeholder)
- Flugregelung: PID-basierter Control-Task
- Kommunikation: Wi-Fi UDP Eingang und WLED-Anzeige
- FSM / Zustandsverwaltung

---

## 1. Setup & Task-Architektur

### `src/main.cpp`
- Ruft `initQueues()` auf, um globale Queues zu erzeugen.
- Ruft `create_tasks()` auf, um die FreeRTOS-Tasks zu starten.
- Die `loop()`-Funktion ist leer und wartet nur, was typisch für ein RTOS-Design ist.

### `src/tasks.cpp`
- Erzeugt die wichtigsten Tasks per `xTaskCreatePinnedToCore()`:
  - `TaskIMU`
  - `TaskControl`
  - `TaskEKF`
  - `TaskGPS`
  - `TaskBaro`
  - `TaskADC`
  - `TaskFSM`
  - `TaskWiFi`
  - `TaskWLED`
- Damit ist die Grundstruktur der parallelen Ausführung im System definiert.

---

## 2. Gemeinsame Daten & Queues

### `include/shared.h` / `src/shared.cpp`
- Globale Queue-Handles sind deklariert für:
  - `imuQueue`
  - `baroQueue`
  - `gpsQueue`
  - `ekfQueue`
  - `inputQueue`
  - `ADCQueue`
  - `fsm_command_queue`
  - `fsm_event_queue`
  - `wled_command_queue`
- Die Queues werden in `initQueues()` erzeugt.
- Das Queue-Layout ist jetzt konsistent für Sensor- und Steuerdaten.

### `include/data_structures.h`
- Definiert das Netzwerksteuerpaket `control_packet_t`
- Definiert `BaroData`, `GPSData` und `EKFState_t`
- `EKFState_t` ist als Platzhalter für Zustandsschätzung hinzugefügt worden.

---

## 3. Sensoren

### `src/tasks/imu_task.cpp`
- Implementiert die BNO055-Initialisierung über UART.
- Liest Quaternion-Daten vom Sensor aus.
- Sendet `IMUSample_t` in `imuQueue`.
- Zustand: weitgehend implementiert, aber es fehlt noch die Umrechnung in Euler/Winkel und ein echter AHRS-/EKF-Algorithmus.

### `src/tasks/baro_task.cpp`
- Implementiert DPS368-Barometer-Treiber inklusive Kalibrierungslesen und Messung.
- Berechnet Druck, Temperatur und Höhe.
- Sendet `BaroData` in `baroQueue`.
- Zustand: bereits funktionale Sensoranbindung vorhanden, jedoch mit TODOs in der Koeffizienten-Extraktion und optionalem Interrupt-Handling.

### `src/tasks/gps_task.cpp`
- Initialisiert UART und liest NMEA über TinyGPS++.
- Validiert Fix/Daten und aktualisiert `gps_data`.
- Sendet `GPSData` in `gpsQueue`.
- Zustand: implementiert, aber es gibt noch Optimierungspunkte und redundante kommentierte Codeblöcke.

### `src/tasks/adc_task.cpp`
- Liest Strom und Spannung aus ADC-Kanälen.
- Glättet Messwerte und sendet `ADCSample_t` in `ADCQueue`.
- Zustand: funktional, zur Batterieüberwachung geeignet.

---

## 4. Zustandsschätzung / EKF

### `src/tasks/ekf_task.cpp`
- Liest latest IMU-, Baro- und GPS-Daten aus den Queues.
- Aktuell nur ein Platzhalter: es gibt noch keine echte EKF-/Sensorfusion.
- Die Ausgabestruktur `EKFState_t` ist vorhanden, wird aber nicht vollständig genutzt.
- Zustand: `TaskEKF` ist funktional als Pipeline-Knoten, aber nicht als echter Filter implementiert.

---

## 5. Steuerung

### `src/tasks/control_task.cpp`
- Implementiert lokale PID- und P-Klassen.
- Liest Steuerbefehle aus `inputQueue`.
- Peeked `ekfQueue`, um aktuelle Pitch/Roll/Yaw/Altitude-Werte zu erhalten.
- Motor-Mixing ist skizziert, aber PWM-Ausgabe und echte Achsregelung fehlen.
- Viele TODOs existieren für die tatsächliche Regelungslogik (Gyro-Feedback, Z-Regler, Spannungs-skalierung, PWM-Ausgabe).
- Zustand: solide Struktur, jedoch nur teilweise funktional.

---

## 6. Kommunikation & UI

### `src/tasks/wifi_task.cpp`
- Stellt einen Wi-Fi Access Point `DRONE_FC` bereit.
- Lauscht auf UDP für `control_packet_t`.
- Prüft Magic-Header und CRC.
- Sendet validierte Pakete an `fsm_command_queue`, `inputQueue` und `wled_command_queue`.
- Zustand: gut implementiert für Steuerpakete.

### `src/tasks/wled_debug.cpp`
- Nutzt NeoPixel-LEDs zur Anzeige von Steuerparametern.
- Visualisiert X/Y/Z/Yaw/Mode.
- Zustand: implementiert und funktional.

---

## 7. FSM / Supervisory Logic

### `src/tasks/fsm_task.cpp`
- Enthält State-Machine-Grundgerüst mit vielen TODOs.
- Zustände und Übergänge sind als Struktur vorhanden.
- Funktionen wie Arming, Takeoff, Altitude Hold, Landing und Failsafe sind nur kommentiert und nicht vollständig implementiert.
- Zustand: Framework ist da, funktionale Logik fehlt noch.

---

## 8. Entwicklungsstatus Gesamt

### Was bereits implementiert ist
- Grundlegende Task-Architektur und Queue-Pipeline
- IMU-, Baro-, GPS- und ADC-Datenerfassung
- Wi-Fi-Kommunikation und Steuerpaketverarbeitung
- WLED-Anzeige
- Placeholder-Pipeline für EKF und Control

### Was noch fehlt / muss entschieden werden
- Echte Zustandsschätzung / sensor fusion / EKF
- tatsächliche Flugregelung inklusive Motor-PWM
- FSM-Arming/Takeoff/Landing-Logik
- stabile Fehlerbehandlung bei Sensorfehlern und Link-Ausfall
- Kalibrierungs- und Filter-Parameter
- Aufgabe der `TaskControl`-Plattform als Inner- vs. Outer-Loop

---

## Empfehlung für die nächsten Schritte
1. EKF-/AHRS-Modul sauber spezifizieren und implementieren
2. Control-Task mit Gyro-Feedback und PWM-Ausgabe vervollständigen
3. FSM mit echten Sicherheitsbedingungen koppeln
4. Queue-Größen und Timeouts prüfen
5. Dokumentation der Pin-Belegung und Versorgung prüfen (siehe `docs/Pinnbelegung.md`)

---

## Fazit
Das Projekt hat eine klare Architektur und viele grundlegende Bausteine stehen. Die größte Lücke ist derzeit die aktive Regelung und Zustandsschätzung: Sensorakquise ist bereits vorhanden, aber die Logik für Flugsteuerung und stabile Fusion ist noch nicht fertig.
