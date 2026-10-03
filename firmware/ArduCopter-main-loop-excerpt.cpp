/*
 * ArduCopter main loop — annotated excerpt
 * ----------------------------------------
 * Source : ArduPilot ArduCopter 3.x (ArduCopter.pde), the firmware flashed to the
 *          APM 2.6 flight controller on this vehicle.
 * License: GNU GPL v3 — Copyright (c) the ArduPilot Dev Team. https://github.com/ArduPilot/ardupilot
 *
 * This file is NOT original code by the project team. It is an excerpt from the
 * project's code appendix, kept here to document how the flight software is structured
 * and where the team's tuning (config/tuned-parameters.csv) takes effect.
 * Comments marked "// [project]" were added for this repository.
 */

// ---------------------------------------------------------------------------
// Scheduler table (APM 2.x build). Every regular task apart from fast_loop()
// is listed with how often it runs (in 10 ms ticks) and its time budget (µs).
//   1 = 100 Hz · 2 = 50 Hz · 4 = 25 Hz · 10 = 10 Hz · 33 = 3 Hz · 100 = 1 Hz
// ---------------------------------------------------------------------------
static const AP_Scheduler::Task scheduler_tasks[] PROGMEM = {
    { rc_loop,               1,   100 },  // read RC receiver (PPM) + mode switch
    { throttle_loop,         2,   450 },  // inertial altitude, land detector
    { update_GPS,            2,   900 },  // uBlox NEO-6M
    { update_batt_compass,  10,   720 },  // battery monitor (FS_BATT_VOLTAGE) + compass
    { read_aux_switches,    10,    50 },  // CH7 / CH8 auxiliary functions
    { arm_motors_check,     10,    10 },
    { auto_trim,            10,   140 },
    { update_altitude,      10,  1000 },  // barometer + sonar (see below)
    { run_nav_updates,       4,   800 },  // waypoint / loiter navigation
    { update_thr_cruise,     1,    50 },
    { three_hz_loop,        33,    90 },  // fence checks
    { compass_accumulate,    2,   420 },
    { barometer_accumulate,  2,   250 },
    { update_notify,         2,   100 },
    { one_hz_loop,         100,   420 },
    { ekf_dcm_check,        10,    20 },
    { crash_check,          10,    20 },
    { gcs_check_input,       2,   550 },  // MAVLink in  (3DR 433 MHz telemetry)
    { gcs_send_heartbeat,  100,   150 },  // MAVLink heartbeat to Mission Planner / DroidPlanner
    { gcs_send_deferred,     2,   720 },
    { gcs_data_stream_send,  2,   950 },
    { update_mount,          2,   450 },
    { ten_hz_logging_loop,  10,   300 },  // dataflash logs used for root-cause analysis
    { fifty_hz_logging_loop, 2,   220 },
    { perf_update,        1000,   200 },
    { read_receiver_rssi,   10,    50 },
};

void setup()
{
    cliSerial = hal.console;

    // Load the default values of variables listed in var_info[]
    AP_Param::setup_sketch_defaults();

    // Storage layout for copter
    StorageManager::set_layout_copter();

    // Serial ports, I2C bus, sensors, parameters from EEPROM
    init_ardupilot();

    // Initialise the main loop scheduler
    scheduler.init(&scheduler_tasks[0],
                   sizeof(scheduler_tasks) / sizeof(scheduler_tasks[0]));
}

void loop()
{
    // Wait for an INS (MPU6000) sample — this paces the 100 Hz loop
    if (!ins.wait_for_sample(1000)) {
        Log_Write_Error(ERROR_SUBSYSTEM_MAIN, ERROR_CODE_MAIN_INS_DELAY);
        return;
    }
    uint32_t timer = micros();

    // Check loop time
    perf_info_check_loop_time(timer - fast_loopTimer);

    // Delta time used by the PI/PID loops
    G_Dt           = (float)(timer - fast_loopTimer) / 1000000.f;
    fast_loopTimer = timer;

    // Main-loop failure monitoring
    mainLoop_count++;

    // Time-critical work first
    fast_loop();

    // One scheduler tick has passed; run every task that is due
    scheduler.tick();
    uint32_t time_available = (timer + MAIN_LOOP_MICROS) - micros();
    scheduler.run(time_available);
}

// Main loop — 100 Hz
static void fast_loop()
{
    // IMU / AHRS attitude estimate
    read_AHRS();

    // Low-level rate controllers (RATE_*_P/I/D in config/tuned-parameters.csv)
    attitude_control.rate_controller_run();

    // Write PWM to the four 30 A ESCs (APM outputs 1–4)
    set_servos_4();

    // Inertial navigation
    read_inertia();

    // Run the active flight mode's attitude controller
    // (Stabilize, AltHold, Loiter, Auto, Guided, RTL, Land …)
    update_flight_mode();

#if OPTFLOW == ENABLED
    if (g.optflow_enabled) {
        update_optical_flow();
    }
#endif
}

// RC input — 100 Hz
static void rc_loop()
{
    read_radio();            // 8-channel receiver via PWM→PPM encoder
    read_control_switch();   // CH5 flight-mode switch
}

// Battery and compass — 10 Hz
static void update_batt_compass(void)
{
    // Read battery before compass: it may be used for motor-interference compensation
    read_battery();

    if (g.compass_enabled) {
        // [project] Motor interference on the compass was measured at about 80 %.
        // The compass was moved away from the motors and COMPASS_DEC set to 2.11° E.
        compass.set_throttle((float)g.rc_3.servo_out / 1000.0f);
        compass.read();
        if (should_log(MASK_LOG_COMPASS)) {
            Log_Write_Compass();
        }
    }

    throttle_integrator += g.rc_3.servo_out;
}

// Barometer and sonar altitude — 10 Hz
static void update_altitude()
{
    // [project] Barometer shielded from prop-wash after Loiter tests
    read_barometer();

    // [project] Sonar used below ~7 m with a low-pass filter added during AltHold tests;
    // the barometer is used above that height
    sonar_alt = read_sonar();

    if (should_log(MASK_LOG_CTUN)) {
        Log_Write_Control_Tuning();
    }
}

AP_HAL_MAIN();
