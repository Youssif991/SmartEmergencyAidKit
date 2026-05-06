#pragma once

/*
  Types.h — Shared data structures for sensor readings and system state.
  
  These structs provide a clean interface for passing aggregated sensor data
  between modules (e.g., main.cpp → display, logging, etc.).
*/

#include <Arduino.h>

// ── Sensor Readings (complete snapshot) ────────────────────────────────────────

struct SensorReadings
{
    // Temperature (MLX90614)
    float objectTemp;
    float ambientTemp;

    // Heart rate (MAX30105 + PBA algorithm)
    int heartRate;
    bool fingerPresent;

    // Blood oxygen (MAX30105 + SpO2 algorithm)
    int spo2;

    // Motion (MPU6050)
    float accelX, accelY, accelZ;  // in g
    float gyroX, gyroY, gyroZ;     // in degrees/second

    // Timestamp
    uint32_t timestamp;
};

// ── IMU Data (accelerometer + gyroscope) ───────────────────────────────────────

struct IMUData
{
    float accelX, accelY, accelZ;  // in g
    float gyroX, gyroY, gyroZ;     // in degrees/second
    float temperature;             // MPU6050 die temp in °C
    uint32_t timestamp;
};

// ── Biometric Data (heart rate + SpO2) ─────────────────────────────────────────

struct BiometricData
{
    int heartRate;
    int spo2;
    bool fingerPresent;
    uint32_t timestamp;
};

// ── Temperature Data ───────────────────────────────────────────────────────────

struct TemperatureData
{
    float objectTemp;
    float ambientTemp;
    uint32_t timestamp;
};
