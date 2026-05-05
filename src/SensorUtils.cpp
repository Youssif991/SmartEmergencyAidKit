/*
  SensorUtils.cpp — Implementation of sensor utility functions.
  
  These functions aggregate data from the various sensor modules into
  structured formats for easier passing between modules and systems.
*/

#include "SensorUtils.h"
#include "Sensors.h"

// ── External sensor objects (declared in main.cpp) ────────────────────────────
extern MAX30105 sensor;
extern TempSensor temp;
extern Oximeter oxygen;
extern HeartBeat heartbeat;
extern IMU imu;
extern OLED display;

// ── Implementations ────────────────────────────────────────────────────────────

SensorReadings gatherSensorReadings()
{
  SensorReadings readings = {};
  
  // Temperature data
  readings.objectTemp = temp.objectTemp();
  readings.ambientTemp = temp.ambientTemp();
  
  // Heart rate data
  readings.heartRate = heartbeat.beatAvg() > 0 ? heartbeat.beatAvg() : 0;
  readings.fingerPresent = heartbeat.fingerPresent();
  
  // Blood oxygen data
  readings.spo2 = oxygen.spo2Avg() > 50 ? oxygen.spo2Avg() : 0;
  
  // IMU data (acceleration in g, gyroscope in dps)
  readings.accelX = imu.accelX();
  readings.accelY = imu.accelY();
  readings.accelZ = imu.accelZ();
  readings.gyroX = imu.gyroX();
  readings.gyroY = imu.gyroY();
  readings.gyroZ = imu.gyroZ();
  
  // Timestamp
  readings.timestamp = millis();
  
  return readings;
}

IMUData gatherIMUData()
{
  IMUData data = {};
  
  data.accelX = imu.accelX();
  data.accelY = imu.accelY();
  data.accelZ = imu.accelZ();
  data.gyroX = imu.gyroX();
  data.gyroY = imu.gyroY();
  data.gyroZ = imu.gyroZ();
  data.temperature = imu.temperature();
  data.timestamp = millis();
  
  return data;
}

BiometricData gatherBiometricData()
{
  BiometricData data = {};
  
  data.heartRate = heartbeat.beatAvg() > 0 ? heartbeat.beatAvg() : 0;
  data.spo2 = oxygen.spo2Avg() > 50 ? oxygen.spo2Avg() : 0;
  data.fingerPresent = heartbeat.fingerPresent();
  data.timestamp = millis();
  
  return data;
}

TemperatureData gatherTemperatureData()
{
  TemperatureData data = {};
  
  data.objectTemp = temp.objectTemp();
  data.ambientTemp = temp.ambientTemp();
  data.timestamp = millis();
  
  return data;
}
