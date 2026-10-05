# Autonomous Quadcopter UAV — Flight Control, Telemetry & FPV — Engineering Guide

Designed, assembled and flight-tested an autonomous quadcopter with GPS waypoint navigation, MAVLink telemetry to a ground control station, failsafe protection, and a live 5.8 GHz video link for remote surveillance.

## Visual overview

![Functional overview](overview/architecture.svg)

*New explanatory diagram; grouped responsibilities, not an as-built schematic or test result.*

![Engineering workflow](overview/workflow.svg)

*New explanatory workflow; a documentation aid, not evidence that every proposed check was performed.*

## Control, telemetry and video

The APM flight controller reads navigation and attitude sensors and drives the propulsion system. RC commands, MAVLink telemetry and the 5.8 GHz FPV stream are separate paths. Keeping those paths explicit explains why a working video link does not prove command or navigation health.

## Testing as an engineering loop

The report-derived material covers calibration, PID tuning and flight-mode testing. Log-driven fixes include IMU re-centring, vibration damping and barometer shielding, sonar filtering and compass placement. Before/after figures should be read with the matching fault discussion in the README.

## Scope of reported values

Propulsion values from eCalc are estimates; telemetry range and flight observations come from the documented tests. Tuned parameters describe this build and are not universal settings for other aircraft. The new overview diagram simplifies the original wiring and does not replace the build documentation.

## Evidence to review or collect

The following are suggested review checks. A checklist entry is not a claimed pass result.

- Sensor and RC calibration.
- Propulsion estimate versus observed behaviour.
- Mode/failsafe test record.
- Before/after logs for each reported fix.

## Source gallery

![design and test flow](images/01-design-and-test-flow.png)

*design and test flow.*

![system block diagram](images/02-system-block-diagram.png)

*system block diagram.*

![ecalc inputs](images/03-ecalc-inputs.png)

*ecalc inputs.*

![ecalc results](images/04-ecalc-results.png)

*ecalc results.*

![ecalc motor efficiency](images/05-ecalc-motor-efficiency.png)

*ecalc motor efficiency.*

![barometer test](images/06-barometer-test.png)

*barometer test.*

![sonar test](images/07-sonar-test.png)

*sonar test.*

![accelerometer test](images/08-accelerometer-test.png)

*accelerometer test.*

![accelerometer calibration cli](images/09-accelerometer-calibration-cli.png)

*accelerometer calibration cli.*

![gps test](images/10-gps-test.jpg)

*gps test.*

![rc calibration](images/11-rc-calibration.png)

*rc calibration.*

![rc calibration summary](images/12-rc-calibration-summary.png)

*rc calibration summary.*

![loiter flowchart](images/14-loiter-flowchart.png)

*loiter flowchart.*

![loiter vibration before](images/15-loiter-vibration-before.png)

*loiter vibration before.*

![loiter vibration after](images/16-loiter-vibration-after.png)

*loiter vibration after.*

![althold flowchart](images/17-althold-flowchart.png)

*althold flowchart.*

![sonar noise before](images/18-sonar-noise-before.png)

*sonar noise before.*

![sonar after low pass](images/19-sonar-after-low-pass.png)

*sonar after low pass.*

![auto heading error](images/20-auto-heading-error.jpg)

*auto heading error.*

![auto heading corrected](images/21-auto-heading-corrected.jpg)

*auto heading corrected.*


## Sources and provenance

- [Published portfolio description](https://mahyoub88.github.io/#proj-quadcopter-uav).
- [Project README](../README.md) and existing repository files.
- [LinkedIn projects](https://www.linkedin.com/in/mohammed-mahyoub/details/projects/): supplementary descriptions and project media.
- New SVG figures and explanatory text were authored for this documentation update; they are not original photographs or new measured results.
