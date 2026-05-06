/**
 * @file imu.h
 * @brief MPU6050 6-axis IMU (Accelerometer + Gyroscope) wrapper
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 1.0
 * @details Wrapper class using libdriver/mpu6050 basic API. Provides throttled readings
 *          with averaging over 4 samples. Outputs acceleration in g and rotation in dps
 */

#pragma once

#include <Arduino.h>
#include "../../Core/Config.h"   // IMU_INTERVAL_MS

// ── Constants ─────────────────────────────────────────────────────────────────

// Number of samples averaged before a new reading is published.
// Reduce to 1 for the raw, unaveraged value.
constexpr uint8_t IMU_AVG_SAMPLES = 4;

class IMU
{
public:
    // ── Lifecycle ──────────────────────────────────────────────────────────────

    // Initialises the MPU6050 over I2C (AD0=GND → address 0x68).
    // Wire must already be started before calling this.
    // Returns false if the sensor is not found.
    bool begin();

    // Should be called every loop(). Only reads the sensor at IMU_INTERVAL_MS.
    void update();

    // ── Results (all in SI / standard units) ──────────────────────────────────

    // Acceleration in g  (1 g ≈ 9.81 m/s²)
    float accelX() const { return _ax; }
    float accelY() const { return _ay; }
    float accelZ() const { return _az; }

    // Angular velocity in degrees/second
    float gyroX()  const { return _gx; }
    float gyroY()  const { return _gy; }
    float gyroZ()  const { return _gz; }

    // Die temperature of the MPU6050 (°C) — not the ambient temperature.
    float temperature() const { return _temp; }

    // True once begin() has succeeded.
    bool  ready()       const { return _ready; }

private:
    float    _ax = 0.0f, _ay = 0.0f, _az = 0.0f;
    float    _gx = 0.0f, _gy = 0.0f, _gz = 0.0f;
    float    _temp  = 0.0f;
    uint32_t _lastMs = 0;
    bool     _ready  = false;

    // Averaging accumulators
    float    _axSum = 0, _aySum = 0, _azSum = 0;
    float    _gxSum = 0, _gySum = 0, _gzSum = 0;
    uint8_t  _sampleCount = 0;
};
