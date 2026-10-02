# Autonomous Quadcopter UAV — Flight Control, Telemetry & FPV

An autonomous quadcopter built around **ArduPilot (APM 2.6)**. It has GPS waypoint navigation, **MAVLink** telemetry to a ground control station, failsafe protection, and a live **5.8 GHz** video link for remote surveillance. The vehicle was assembled, calibrated, tuned and flight-tested across autonomous flight modes, and faults were root-caused from flight logs.

> Team project covering propulsion sizing, airframe and electronics integration, sensor calibration, PID tuning, flight-mode and failsafe testing, telemetry and video systems, and technical documentation.

**Stack:** ArduPilot / ArduCopter · APM 2.6 · MAVLink · Mission Planner · DroidPlanner · 3DR Radio · uBlox GPS · PID control · eCalc

---

## System architecture

```
 RC Tx 2.4 GHz ──► Rx ──► PPM encoder ──┐
                                        ▼
 GPS + compass ─────────────────► APM 2.6 (MPU6000 IMU, barometer) ──► 4× ESC 30A ──► 4× BLDC motors
 Sonar · optical flow ─────────────────┘        │
                                                ├──► 3DR 433 MHz telemetry ◄──► GCS (Mission Planner / phone)
 Camera ──► MinimOSD ──► 5.8 GHz video Tx ─────────────────────────────► Rx ──► laptop display
 3S LiPo ──► power module (5 V to APM) + power distribution to ESCs
```

## Hardware

| Subsystem | Component |
|---|---|
| Frame | DJI F450 |
| Propulsion | 4× DJI 2212/920KV brushless motors, 4× 30A ESCs |
| Power | 3S LiPo, APM power module, power distribution board |
| Flight controller | ArduPilot Mega APM 2.6 (MPU6000 IMU, barometer) |
| Navigation | uBlox NEO-6M GPS with compass |
| Altitude / position aids | Sonar (≤ 7 m), optical-flow sensor |
| Telemetry | 3DR Radio 433 MHz, MAVLink (≈ 400 m range tested) |
| Control link | 2.4 GHz RC with PWM→PPM encoder |
| Video | Camera, MinimOSD, 5.8 GHz video transmitter/receiver |

**Propulsion sizing (eCalc):**

- All-up weight ≈ 1.6 kg.
- Propeller options were compared for hover throttle and flight time (estimated hover ≈ 10–15 min depending on propeller).
- Motor efficiency was checked against the operating current.

## Integration & calibration

- Accelerometer, compass, RC and ESC calibration in Mission Planner.
- 3DR telemetry configured for frequency and data rate.
- Motor rotation order and directions verified.
- Stabilize-mode PID tuning (rate and stabilize gains).

## Flight testing

| Mode | What was verified |
|---|---|
| Stabilize | Manual flight with self-levelling after PID tuning |
| Loiter | Position, heading and altitude hold |
| AltHold | Constant altitude using barometer and sonar |
| Auto | Waypoint missions planned on the map in Mission Planner |
| Guided | Fly-to-point commands from the GCS |
| Follow-Me | Tracking a phone-based GCS over 3DR telemetry |
| Failsafes | Radio loss → land in place; geofence → return-to-launch (land if GPS lost) |

## Root-cause findings from flight logs

| Symptom | Cause | Fix |
|---|---|---|
| Drift and a crash in Stabilize | IMU (MPU6000 on the APM) mounted away from the centre of gravity | Re-centred the controller, then re-tuned the PIDs |
| Poor position hold in Loiter | Vibration corrupting accelerometer data; barometer disturbed by airflow | Vibration-damping foam under the APM; shielded the barometer |
| Noisy altitude in AltHold | Sonar noise from motors, ESCs and propellers | Low-pass filtering on sonar; sonar used ≤ 7 m, barometer above |
| Heading error in Auto | ≈ 80% magnetic interference from motors, plus uncorrected declination | Moved the compass away from the motors, set local declination (2.11°E), recalibrated |

## Related study

The project draws on a study of UAS communications:
- command-and-control and telemetry links over GSM/GPRS;
- line-of-sight bands from 900 MHz to 5.8 GHz;
- UAS spectrum requirements (ITU-R M.2171).

---

**Author:** Mohammed Mahyoub · [Portfolio](https://mahyoub88.github.io/) · [LinkedIn](https://www.linkedin.com/in/mohammed-mahyoub/) · [ORCID](https://orcid.org/0009-0003-5640-352X)
