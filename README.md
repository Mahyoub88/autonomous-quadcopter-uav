# Autonomous Quadcopter UAV — Flight Control, Telemetry & FPV

**Project author and sole implementer:** Mohammed Mahyoub.

An autonomous quadcopter built around **ArduPilot (APM 2.6)**. It flies GPS waypoint missions, sends **MAVLink** telemetry to a ground control station, protects itself with radio and geofence failsafes, and streams live **5.8 GHz** video for remote surveillance.

I assembled, calibrated, tuned and flight-tested the vehicle across seven flight modes. Every flight fault was root-caused from the dataflash logs and fixed on the vehicle.

> **Independent project.** The work covered propulsion sizing, airframe and electronics integration, sensor calibration, PID tuning, flight-mode and failsafe testing, telemetry and video systems, and technical documentation.

**Stack:** ArduPilot / ArduCopter · APM 2.6 (ATmega2560 + MPU6000) · MAVLink · Mission Planner · DroidPlanner · 3DR Radio 433 MHz · uBlox NEO-6M GPS · PID control · eCalc

---

## Contents

1. [Repository layout](#repository-layout)
2. [Design and test process](#design-and-test-process)
3. [System architecture](#system-architecture)
4. [Hardware](#hardware)
5. [Propulsion sizing (eCalc)](#propulsion-sizing-ecalc)
6. [Component tests and calibration](#component-tests-and-calibration)
7. [Flight software](#flight-software)
8. [PID tuning — final values](#pid-tuning--final-values)
9. [Flight testing and root-cause fixes](#flight-testing-and-root-cause-fixes)
10. [Failsafe testing](#failsafe-testing)
11. [Video / FPV system](#video--fpv-system)
12. [Results summary](#results-summary)
13. [Related study](#related-study)

---

## Repository layout

```
autonomous-quadcopter-uav/
├── README.md                               ← this document
├── config/
│   └── tuned-parameters.csv                ← final PID gains and failsafe parameters
├── firmware/
│   └── ArduCopter-main-loop-excerpt.cpp    ← annotated ArduCopter main loop (GPLv3, ArduPilot)
└── docs/
    └── images/                             ← test screenshots, logs and diagrams
```

---

## Design and test process

The build ran in two stages. The first stage covered design and construction. The second covered full integration and flight testing. Each stage gate had to pass before moving on.

<p align="center"><img src="docs/images/01-design-and-test-flow.png" width="420" alt="Design and test flow chart"></p>

---

## System architecture

<p align="center"><img src="docs/images/02-system-block-diagram.png" width="720" alt="System block diagram"></p>

```
 RC Tx 2.4 GHz ──► Rx ──► PWM→PPM encoder ──┐
                                            ▼
 GPS + compass ─────────────────────► APM 2.6 (MPU6000 IMU, barometer) ──► 4× ESC 30A ──► 4× BLDC motors
 Sonar · optical flow ─────────────────┘        │
                                                ├──► 3DR 433 MHz telemetry ◄──► GCS (Mission Planner / phone)
 Camera ──► MinimOSD ──► 5.8 GHz video Tx ─────────────────────────────► Rx ──► laptop / TV
 3S LiPo ──► power module (5 V to APM) + power distribution board to ESCs
```

- **Power.** A 3-cell LiPo feeds the power distribution board, which powers the four ESCs. The board also routes 5 V to the APM through its power module. That 5 V supply runs the controller, its on-board sensors and the external GPS/compass.
- **Propulsion.** Each ESC drives its motor through three bullet connectors. The ESC inputs connect to APM outputs 1–4.
- **Sensors.** The sonar sits on analog input A0. The optical-flow sensor holds position close to the ground. A buzzer on A5 gives arming and status tones.
- **Ground link.** The 3DR radio connects the vehicle to the laptop GCS. Mission Planner configures the vehicle and plans its waypoint missions.

---

## Hardware

| Subsystem | Component |
|---|---|
| Frame | DJI F450 |
| Propulsion | 4× DJI 2212/920KV brushless motors, 4× 30A ESCs |
| Power | 3S LiPo, APM power module, power distribution board |
| Flight controller | ArduPilot Mega APM 2.6 (ATmega2560, MPU6000 IMU, barometer) |
| Navigation | uBlox NEO-6M GPS with compass |
| Altitude / position aids | Sonar (used below 7 m), optical-flow sensor |
| Telemetry | 3DR Radio 433 MHz, MAVLink (≈ 400 m range tested) |
| Control link | 8-channel 2.4 GHz RC with PWM→PPM encoder |
| Video | Camera, MinimOSD, 5.8 GHz 200 mW video transmitter/receiver |

---

## Propulsion sizing (eCalc)

I entered the frame, battery, 30A ESCs (measured at 32 g each), DJI 2212/920KV motors and propellers into eCalc xcopterCalc. I then compared propeller options at the same all-up weight:

| Propeller | All-up weight | Hover throttle | Est. hover time | Notes |
|---|---|---|---|---|
| 10 × 4.7 | 1635 g | 66 % | ≈ 10.3 min | Hover throttle too high (target ≈ 50 %) |
| 13 × 4 | 1635 g | 35 % | ≈ 13 min | Max power 151 W vs 195.6 W motor rating |
| Larger prop | 1635 g | 50 % | ≈ 14.7 min | 122.1 W |

eCalc results are ±10 %. The motor-efficiency curve shows a sweet spot: efficiency rises from about 2.3 A, peaks near 8 A and holds steady to about 12 A. This chart was used to check the hover operating point.

<p align="center">
<img src="docs/images/03-ecalc-inputs.png" width="49%" alt="eCalc inputs">
<img src="docs/images/04-ecalc-results.png" width="49%" alt="eCalc results">
</p>
<p align="center"><img src="docs/images/05-ecalc-motor-efficiency.png" width="560" alt="Motor efficiency chart"></p>

---

## Component tests and calibration

Each component was tested on its own before integration.

### Barometer and sonar

The barometer readings drifted with the weather and airflow. The sonar was steady at short range (readings held at 43–44 cm). From these tests I set the altitude strategy used in flight:

- below 7 m, sonar plus barometer;
- above 7 m, barometer only.

<p align="center">
<img src="docs/images/06-barometer-test.png" width="56%" alt="Barometer test output">
<img src="docs/images/07-sonar-test.png" width="42%" alt="Sonar test output">
</p>

### Accelerometer

The MPU6000 accelerometer was tested from the ArduCopter command line, then calibrated in six orientations: level, left, right, nose down, nose up and back. The calibration produced offsets of (−0.07, 0.19, 0.09) and a scaling of 0.99 on all three axes.

<p align="center">
<img src="docs/images/08-accelerometer-test.png" width="54%" alt="Accelerometer raw test">
<img src="docs/images/09-accelerometer-calibration-cli.png" width="44%" alt="Accelerometer calibration in CLI">
</p>

### GPS

The GPS showed its position on the map. Indoors it did not reach a 3D fix: the blue LED stayed blinking. It locked once the vehicle was outdoors.

<p align="center"><img src="docs/images/10-gps-test.jpg" width="640" alt="GPS test"></p>

### RC transmitter and receiver

The RC link is an 8-channel radio feeding a PPM encoder:

- CH1 roll
- CH2 pitch
- CH3 throttle
- CH4 yaw
- CH5 flight mode
- CH6 in-flight tuning
- CH7 land
- CH8 RTL

The channels were calibrated in Mission Planner to roughly 1100–1900 µs.

<p align="center">
<img src="docs/images/11-rc-calibration.png" width="60%" alt="RC calibration">
<img src="docs/images/12-rc-calibration-summary.png" width="36%" alt="RC calibration summary">
</p>

### 3DR telemetry, ESCs and motors

- **Telemetry.** The 3DR radios on the vehicle and the ground station were set to the same frequency and data rate. I picked the band that had the least noise. Measured range was about 400 m.
- **ESCs.** Each ESC was calibrated to the receiver's throttle range (full stick, connect battery, two beeps, zero throttle, confirmation tone).
- **Motors.** Motor order and spin direction were verified. M1 and M3 spin counter-clockwise; M2 and M4 spin clockwise, so their torques balance.


---

## Flight software

The APM 2.6 runs **ArduCopter** firmware. Mission Planner on the laptop and DroidPlanner on the phone talk to it over MAVLink.

- `setup()` initialises the board, loads parameters from EEPROM and starts the scheduler.
- `loop()` runs at 100 Hz, paced by MPU6000 samples. It calls `fast_loop()` for attitude, the rate controllers and motor output, then runs the scheduled tasks: RC input, GPS, barometer/sonar, navigation, telemetry and logging.

See the annotated excerpt in [`firmware/ArduCopter-main-loop-excerpt.cpp`](firmware/ArduCopter-main-loop-excerpt.cpp). It is ArduPilot code (GPLv3), included to show where my tuning and fixes take effect.

```cpp
// Main loop — 100 Hz
static void fast_loop()
{
    read_AHRS();                               // IMU / attitude estimate
    attitude_control.rate_controller_run();    // RATE_*_P/I/D gains
    set_servos_4();                            // PWM to the four ESCs
    read_inertia();                            // inertial navigation
    update_flight_mode();                      // Stabilize / AltHold / Loiter / Auto …
}

// Barometer and sonar altitude — 10 Hz
static void update_altitude()
{
    read_barometer();          // shielded from prop-wash after Loiter tests
    sonar_alt = read_sonar();  // low-pass filtered, used below ~7 m
}
```

---

## PID tuning — final values

Tuning started in Stabilize mode, because every other flight mode builds on it. The usual order was followed:

1. Raise P until the vehicle oscillates, then back off.
2. Raise I until it holds its attitude without drifting, checked in wind.
3. Add D, then re-trim P and I.

The final values (full list in [`config/tuned-parameters.csv`](config/tuned-parameters.csv)):

| Loop | P | I | D | IMAX |
|---|---|---|---|---|
| Stabilize (angle) roll / pitch / yaw | 4.50 / 4.50 / 4.50 | – | – | – |
| Rate roll | 0.15 | 0.10 | 0.004 | 1000 |
| Rate pitch | 0.15 | 0.10 | 0.004 | 500 |
| Rate yaw | 0.20 | 0.02 | 0.00 | 1000 |
| Altitude hold | 1.00 | – | – | – |
| Throttle rate | 6.00 | – | – | – |
| Throttle accel | 0.75 | 1.50 | 0.00 | 500 |

| Parameter | Value |
|---|---|
| `FS_BATT_VOLTAGE` | 9.2 V |
| `RTL_ALT_FINAL` | 15 m |
| `LAND_SPEED` | 40 cm/s |
| `RC_FEEL` | 100 |
| Stabilize smoothing gain | 12 (crisp) |
| Body-frame rate feed-forward | enabled |

---

## Flight testing and root-cause fixes

| Mode | What was verified |
|---|---|
| Stabilize | Manual flight with self-levelling after PID tuning |
| Loiter | Position, heading and altitude hold |
| AltHold | Constant altitude using barometer and sonar |
| Auto | Waypoint missions planned on the map in Mission Planner |
| Guided | Fly-to-point commands from the GCS |
| Follow-Me | Tracking a phone-based GCS (DroidPlanner) over 3DR telemetry, position sent every 5 s |
| Failsafes | Radio loss → land in place; geofence → return-to-launch (land if GPS lost) |

### 1 · Stabilize — drift and crash

- **Symptom:** a strong tilt in flight, ending in a crash that broke a landing skid.
- **Cause:** the MPU6000 IMU on the APM was mounted away from the vehicle's centre of gravity.
- **Fix:** re-centred the controller on the frame and re-tuned the PIDs to the values above. The problem did not return.

### 2 · Loiter — poor position hold

- **Symptom:** the vehicle would not hold its position.
- **Cause:** the logs showed vibration corrupting the accelerometer data. The barometer was also disturbed by prop-wash.
- **Fix:** put about 2 cm of damping foam under the APM, then doubled it to about 4 cm, and shielded the barometer. Vibration dropped and the barometer readings settled.

<p align="center">
<img src="docs/images/15-loiter-vibration-before.png" width="49%" alt="Loiter vibration before">
<img src="docs/images/16-loiter-vibration-after.png" width="49%" alt="Loiter vibration after damping">
</p>
<p align="center"><sub>Left: vibration in Loiter before damping · Right: after foam damping and barometer shielding</sub></p>

<details>
<summary>Loiter mode logic (flow chart)</summary>
<p align="center"><img src="docs/images/14-loiter-flowchart.png" width="420" alt="Loiter flow chart"></p>
</details>

### 3 · AltHold — noisy altitude

- **Symptom:** the altitude reading was noisy and unreliable.
- **Cause:** the sonar was picking up noise from the motors, ESCs and propellers.
- **Fix:** added low-pass filtering on the sonar. Below 7 m the vehicle uses sonar and barometer together; above 7 m it uses the barometer only.

<p align="center">
<img src="docs/images/18-sonar-noise-before.png" width="49%" alt="Sonar noise before filter">
<img src="docs/images/19-sonar-after-low-pass.png" width="49%" alt="Sonar after low-pass filter">
</p>
<p align="center"><sub>Left: sonar noise in AltHold · Right: barometer and sonar after the low-pass filter</sub></p>

<details>
<summary>AltHold mode logic (flow chart)</summary>
<p align="center"><img src="docs/images/17-althold-flowchart.png" width="380" alt="AltHold flow chart"></p>
</details>

### 4 · Auto — heading error on waypoint missions

- **Symptom:** the heading was wrong while flying waypoint missions.
- **Cause:** magnetic interference from the motors, measured at about 80 %, plus magnetic declination that had not been corrected.
- **Fix:** moved the compass away from the motors, set the local declination (2.11° E) and recalibrated the compass. After that the vehicle tracked the mission correctly.

<p align="center">
<img src="docs/images/20-auto-heading-error.jpg" width="49%" alt="Auto mode heading error">
<img src="docs/images/21-auto-heading-corrected.jpg" width="49%" alt="Auto mode after correction">
</p>
<p align="center"><sub>Left: heading error on the waypoint mission · Right: after compass relocation, declination and recalibration</sub></p>

---

## Failsafe testing

- **Radio failsafe.** The transmitter was switched off in flight several times. Each time, the vehicle landed where the signal was lost.
- **Fence failsafe.** A circular radius and a maximum altitude were set around the take-off point. When the vehicle reached the fence, it switched to RTL and returned home, provided GPS was good. Without GPS it landed instead.
- **Battery failsafe.** The trigger was set at 9.2 V (`FS_BATT_VOLTAGE`).

---

## Video / FPV system

The video chain runs from the camera through the MinimOSD to the 5.8 GHz transmitter, then to the receiver and finally the display:

- I built a camera video-out cable from a 10-pin mini-USB connector with a 100 kΩ resistor. The resistor switches the camera into analog video-out mode.
- The MinimOSD overlays telemetry data on the live video.
- The transmitter and receiver were set to the same channel (channel 3 of 8).
- Live video was verified in two stages: first on a TV, then on a laptop through an AV-to-USB capture device.

---

## Results summary

- Flew in Stabilize, AltHold, Loiter, Auto, Guided and Follow-Me, with radio, fence and battery failsafes in place.
- Four in-flight faults were root-caused from logs and fixed on the vehicle: IMU placement, vibration, sonar noise and compass interference.
- Measured telemetry range was about 400 m, with live FPV video and OSD.
- Possible extensions:
  - solar-assisted endurance
  - face-recognition follow mode
  - an antenna tracker for longer range
  - a mapping camera for 3D maps

## Related study

The project draws on a study of UAS communications:

- command-and-control and telemetry links over GSM/GPRS;
- line-of-sight bands from 900 MHz to 5.8 GHz;
- UAS spectrum requirements (ITU-R M.2171).

---

**Author:** Mohammed Mahyoub · [Portfolio](https://mahyoub88.github.io/) · [LinkedIn](https://www.linkedin.com/in/mohammed-mahyoub/) · [ORCID](https://orcid.org/0009-0003-5640-352X)

*Flight software: [ArduPilot](https://github.com/ArduPilot/ardupilot) (GPLv3). Excerpts in `firmware/` keep their original licence.*
