/**
 * @file main.cpp
 * @author Youssef Mohammed, Youssef Hisham, Mahmoud Abdeldayem
 * @brief Program entry point - Optimized for non-blocking sensor flow
 */

#include "Sensors.h"
#include "app.h"

// Module instances
MAX30105 sensor;
TempSensor temp;
Oximeter oxygen;
HeartBeat heartbeat;
IMU imu;
OLED display;

static uint32_t lastIntervalMs = 0;
static uint32_t lastOledMs = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000); // Reduced delay for faster booting

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Initialize IMU
  Wire.setClock(100000);
  if (!imu.begin())
  {
    Serial.println("IMU Init Failed!");
  }

  // Initialize Oximeter
  oxygen.begin(sensor);

  // Initialize Temp
  Wire.setClock(50000);
  temp.begin();

  // Finalize bus speed for the main loop
  Wire.setClock(400000);

  if (display.begin())
  {
    display.printText("SmartAidKit", 30, 10);
    display.display();
  }

  bleInit();
}

void loop()
{
  // 1. CRITICAL: These must run as fast as possible to prevent FIFO overflow
  temp.update();
  imu.update();

  sensor.check();
  while (sensor.available())
  {
    const uint32_t ir = sensor.getFIFOIR();
    const uint32_t red = sensor.getFIFORed();
    sensor.nextSample();

    oxygen.process(ir, red);
    heartbeat.process(ir);
  }

  // 2. TIMED OUTPUT: Only Serial/BLE/OLED updates happen periodically
  if (millis() - lastIntervalMs >= PRINT_INTERVAL_MS)
  {
    lastIntervalMs = millis();

    // Serial Logging
    Serial.printf("Temp Obj: %.1f | HR: %d | SpO2: %d\n",
                  temp.objectTemp(), heartbeat.beatAvg(), oxygen.spo2Avg());

    // BLE Update (Now inside the non-blocking timer)
    sendBLE(
        imu.accelX(), imu.accelY(), imu.accelZ(),
        imu.gyroX(), imu.gyroY(), imu.gyroZ(),
        heartbeat.beatAvg(), oxygen.spo2Avg(),
        temp.objectTemp(), temp.ambientTemp());
  }

  // 3. OLED Update (Slightly different frequency if desired)
  if (millis() - lastOledMs >= OLED_INTERVAL_MS)
  {
    lastOledMs = millis();
    SensorReadings readings = gatherSensorReadings();
    display.printSensorDataStruct(readings);
  }

  yield();
}
