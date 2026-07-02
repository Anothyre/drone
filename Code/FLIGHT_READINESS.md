# Flight Readiness Notes

Ziel: schnell und bewusst entscheiden, was wirklich noch noetig ist, um vom aktuellen Firmware-Stand zu einem sinnvollen ersten Flugtest zu kommen.

## Aktueller Zustand

- `src/tasks.cpp`: Aktuell wird nur `TaskGPS` gestartet. IMU, Control, EKF, Baro, ADC, FSM und WiFi sind auskommentiert.
- `src/tasks/control_task.cpp`: Der Controller enthaelt noch Platzhalter (`PLACEHOLDER`) und `scaleOutput()` gibt konstant `42.0f` zurueck.
- `src/tasks/ekf_task.cpp`: Sensorwerte werden gelesen, aber kein `EKFState_t` in `ekfQueue` veroeffentlicht.
- `src/tasks/fsm_task.cpp`: Self-tests sind Dummy-true, Batterie wird simuliert, Position wird nicht aus EKF/Baro aktualisiert.
- `src/hardware.cpp`: Die Boot-Sequenz aktiviert Motoren und gibt 5 Sekunden Test-Throttle. Das ist fuer Props-off Tests ok, fuer normale Firmware riskant.
- `src/tasks/wifi_task.cpp`: UDP-Controlpfad existiert und schickt Pakete an FSM, Control und WLED. Timeout erzeugt `EV_EXCEPTION`.

## Was wirklich noetig ist

### 1. Sicherer Motorzustand

Muss vor jedem Test mit montierten Props erledigt sein.

- Boot-Motortest hinter einen expliziten Compile-Flag/Testmodus setzen oder entfernen.
- Motors default auf minimum/off halten, nicht idle/throttle, solange nicht bewusst armed.
- Arm/Disarm eindeutig pruefen: Disarm muss sofort `ESC_THROTTLE_MIN` setzen.
- Output limitieren, z.B. erster Test maximal 10-15 Prozent normalized throttle.

Shortcut: fuer Props-off Bench-Test darf Boot-Motortest bleiben, aber nur wenn das bewusst der Test ist.

### 2. Task-Kette aktivieren

Minimal fuer kontrollierten Test:

- `TaskIMU`: BNO055 muss stabil Daten liefern.
- `TaskWiFi`: Control-Pakete muessen ankommen.
- `TaskFSM`: Arming/Disarming und Timeout-Failsafe.
- `TaskControl`: Motoroutput nur wenn armed und frisches Control-Paket vorhanden.

Spaeter:

- `TaskBaro`, `TaskADC`, `TaskLogger`, `TaskWLED`, `TaskGPS`.

Shortcut: fuer ersten Stabilisierungstest kein GPS, kein Baro, kein ADC, kein Logger.

### 3. Minimaler Attitude-State statt voller EKF

Freier Flug braucht keine echte GPS/Baro-Fusion am Anfang. Fuer den ersten Test reicht:

- BNO055 Quaternion in Roll/Pitch/Yaw umrechnen.
- `EKFState_t` mit `valid_attitude=true` in `ekfQueue` schreiben.
- `valid_altitude=false`, `valid_position=false` lassen.

Shortcut: `TaskEKF` kann erstmal eher `TaskAttitudeBridge` sein. Name kann spaeter sauber werden.

### 4. Control auf Stabilize reduzieren

Der aktuelle Controller versucht schon Hoehe, Attitude und Rates zu mischen, hat aber keine echten Inputs fuer vieles.

Minimal:

- `latest.z` als manuelles Throttle interpretieren, nicht als Hoehenziel.
- `latest.x`/`latest.y` als Roll/Pitch Setpoints in kleinem Winkelbereich interpretieren.
- `latest.yaw` optional erstmal ignorieren oder stark limitieren.
- Keine Z-Speed PID, keine Altitude PID, keine Position.
- Motor-Mixer nach jedem Schritt clampen und global begrenzen.

Shortcut: Fuer ersten Lift-Test nur manuelles Throttle plus Level-Stabilisierung auf Roll/Pitch.

### 5. Failsafe muss brutal einfach sein

Vor Props-on:

- Kein frisches Paket innerhalb `CONTROL_TIMEOUT_MS`: disarm/off.
- Ungueltiger Attitude-State: disarm/off.
- Ungueltige Queue oder Sensorfehler: off.
- Expliziter Disarm-Befehl: off.

Shortcut: Lieber sofort Motor aus als "kontrolliert landen". Landing-Logik ist spaeter.

## Was warten kann

- GPS-Fix und GPS-Qualitaet.
- Position hold.
- Mission mode.
- Takeoff automation.
- Landing automation.
- Echter EKF.
- Batterie-Prozentmodell.
- Logger/Telemetry-Polish.
- WLED-Visualisierung.

## Empfohlene Reihenfolge

1. Boot-Motortest absichern.
2. `create_tasks()` fuer Minimalmodus aktivieren: IMU, WiFi, FSM, Control, optional EKF/AttitudeBridge.
3. BNO055 Daten bis `ekfQueue` bringen und seriell verifizieren.
4. Control auf manuellen Throttle plus Roll/Pitch-Stabilisierung umbauen.
5. Props-off Motor-Mixer testen.
6. Ohne Props: Arm, Throttle, Disarm, Link-Timeout testen.
7. Mit Props, festgeschnallt: sehr niedriger Throttle, nur Reaktion auf Kippen pruefen.
8. Erst danach kurzer Hop, keine GPS/Baro-Abhaengigkeit.

## Nicht verhandelbare Go/No-Go Punkte

- No-Go, wenn irgendein Motor beim Boot ungewollt hochlaeuft.
- No-Go, wenn Disarm nicht sofort alle Motoren stoppt.
- No-Go, wenn Control ohne frisches Paket weiterlaeuft.
- No-Go, wenn Attitude fehlt oder stale ist.
- No-Go, wenn Motor-Reihenfolge oder Drehrichtung nicht physisch bestaetigt ist.
- No-Go, wenn Props fuer Softwaretests montiert sind.

## Praktischer Shortcut-Entscheid

Der schnellste realistische Weg ist kein autonomer Flug. Der schnellste Weg ist:

`Manual throttle + BNO055 roll/pitch leveling + UDP arm/disarm + timeout kill`

Alles andere ist fuer den ersten Hop Ballast.
