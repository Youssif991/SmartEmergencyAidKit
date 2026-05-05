#pragma once

/*
  SensorUtils.h — Helper functions for aggregating and processing sensor data.
  
  Provides utility functions for gathering readings from all sensors into
  structured formats, formatting data for output, and other sensor-related
  helper operations.
*/

#include "Types.h"

// ── Sensor Data Gathering ──────────────────────────────────────────────────────

// Gather all current sensor readings into a single SensorReadings struct.
// Call this before updating displays or logging data.
// Requires all sensor modules to be initialized and updated first.
SensorReadings gatherSensorReadings();

// Gather just IMU data into an IMUData struct.
IMUData gatherIMUData();

// Gather just biometric data (heart rate + SpO2) into a BiometricData struct.
BiometricData gatherBiometricData();

// Gather just temperature data into a TemperatureData struct.
TemperatureData gatherTemperatureData();
