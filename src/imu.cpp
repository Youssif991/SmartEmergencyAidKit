/*
  imu.cpp — MPU6050 wrapper using the libdriver/mpu6050 basic API.

  Clock management follows the same convention as temperature.cpp:
    - update() sets Wire to its required speed before reading
    - update() restores Wire to 400 kHz after reading so the
      MAX30105 (which runs at 400 kHz) is never left at the wrong speed.
*/

#include "imu.h"
#include <Wire.h>

extern "C" {
#include "driver_mpu6050_basic.h"
}

// ── Public ────────────────────────────────────────────────────────────────────

bool IMU::begin()
{
    // Clock is already set to 400 kHz by main.cpp before calling begin().
    // MPU6050 supports up to 400 kHz — no change needed here.
    if (mpu6050_basic_init(MPU6050_ADDRESS_AD0_LOW) != 0) {
        Serial.println("[IMU] MPU6050 not found — check wiring and AD0 pin.");
        return false;
    }
    _ready = true;
    Serial.println("[IMU] MPU6050 ready.");
    return true;
}

void IMU::update()
{
    if (!_ready) return;
    if (millis() - _lastMs < IMU_INTERVAL_MS) return;
    _lastMs = millis();

    // Set bus speed for MPU6050 (400 kHz), matching the pattern in
    // TempSensor::update() which does setClock(50000) → read → setClock(400000).
    Wire.setClock(400000);

    float g[3]   = {};
    float dps[3] = {};

    if (mpu6050_basic_read(g, dps) != 0) {
        Serial.println("[IMU] Read error.");
        Wire.setClock(400000);   // restore even on error
        return;
    }

    _axSum += g[0];  _aySum += g[1];  _azSum += g[2];
    _gxSum += dps[0]; _gySum += dps[1]; _gzSum += dps[2];
    _sampleCount++;

    if (_sampleCount >= IMU_AVG_SAMPLES) {
        float n = (float)_sampleCount;
        _ax = _axSum / n;  _ay = _aySum / n;  _az = _azSum / n;
        _gx = _gxSum / n;  _gy = _gySum / n;  _gz = _gzSum / n;
        _axSum = _aySum = _azSum = 0;
        _gxSum = _gySum = _gzSum = 0;
        _sampleCount = 0;
    }

    float deg = 0.0f;
    if (mpu6050_basic_read_temperature(&deg) == 0)
        _temp = deg;

    // Restore to 400 kHz for MAX30105 (same pattern as TempSensor::update()
    // which restores after the slow MLX read).
    Wire.setClock(400000);
}