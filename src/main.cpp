/**
 * @file main.cpp
 * @brief Program entry point and main application loop
 * @author Youssef Mohammed, Youssef Hisham, Mahmoud Abdeldayem
 * @date 2026-05-06
 * @version 1.0
 * @details Initializes all sensors and runs the main event loop.
 *          Manages I2C clock switching, sensor polling, and display updates
 */

#include "Sensors.h"
#include "app.h"
// ── Module instances ──────────────────────────────────────────────────────────
MAX30105 sensor;
TempSensor temp;
Oximeter oxygen;
HeartBeat heartbeat;
IMU imu;
OLED display;

static uint32_t lastPrintMs = 0;
static uint32_t lastOledMs = 0;

// ─────────────────────────────────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);
  delay(10000);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // 1. Start with the IMU at a conservative speed
  Wire.setClock(100000);
  delay(100);
  if (!imu.begin())
  {
    Serial.println("IMU failed at 100kHz, retrying at 400kHz...");
    Wire.setClock(400000);
    if (!imu.begin())
      while (1)
        ;
  }

  // 2. Then do the Oximeter
  Wire.setClock(400000);
  oxygen.begin(sensor);

  // 3. Then the slow Temp sensor
  Wire.setClock(50000);
  temp.begin();

  // 4. Finally set back to 400kHz for the main loop
  Wire.setClock(400000);

  // 5. Initialize OLED display
  if (!display.begin())
  {
    Serial.println("[OLED] Failed to initialize display");
  }
  else
  {
    Serial.println("[OLED] Display initialized");
    display.printText("SmartAidKit", 30, 10);
    display.display();
    delay(1000);
  }

  bleInit();

}
// ─────────────────────────────────────────────────────────────────────────────
void loop()
{
  temp.update();
  imu.update();

  // Drain sensor FIFO and feed each sample to both modules.
  sensor.check();
  while (sensor.available())
  {
    const uint32_t ir = sensor.getFIFOIR();
    const uint32_t red = sensor.getFIFORed();
    sensor.nextSample();

    oxygen.process(ir, red);
    heartbeat.process(ir);
  }

  // ── Periodic Serial output ────────────────────────────────────────────────
  if (millis() - lastPrintMs < PRINT_INTERVAL_MS)
  {
    yield();
    return;
  }
  lastPrintMs = millis();

  Serial.printf("Temp: %.1f C (obj)  %.1f C (amb)  |  ",
                temp.objectTemp(), temp.ambientTemp());

  // Heart rate
  Serial.print("HR: ");
  if (!heartbeat.fingerPresent())
    Serial.print("--");
  else if (heartbeat.beatAvg() > 0)
    Serial.printf("%d bpm", heartbeat.beatAvg());
  else
    Serial.print("calculating...");

  Serial.print("  |  SpO2: ");

  // Blood oxygen
  switch (oxygen.state())
  {
  case OximeterState::NoFinger:
    Serial.println("--");
    break;
  case OximeterState::Filling:
    Serial.println("--");
    break;
  case OximeterState::Streaming:
    if (oxygen.spo2Avg() > 50)
      Serial.printf("%d%%\n", oxygen.spo2Avg());
    else
      Serial.println("calculating...");
    break;
  }

  // ── Update OLED display (always, less frequently to save cycles) ─────────────
  // Runs regardless of SpO2 state so the display shows "--" when there
  // is no finger or while the algorithm is still collecting data.
  if (millis() - lastOledMs >= OLED_INTERVAL_MS)
  {
    lastOledMs = millis();
    SensorReadings readings = gatherSensorReadings();
    display.printSensorDataStruct(readings);
  }

  // IMU output
  if (imu.ready())
  {
    Serial.printf("IMU: Accel(g) X=%.2f Y=%.2f Z=%.2f  |  "
                  "Gyro(dps) X=%.1f Y=%.1f Z=%.1f\n",
                  imu.accelX(), imu.accelY(), imu.accelZ(),
                  imu.gyroX(), imu.gyroY(), imu.gyroZ());
  }


      sendBLE(
  imu.accelX(), imu.accelY(), imu.accelZ(),
  imu.gyroX(),  imu.gyroY(),  imu.gyroZ(),
  heartbeat.beatAvg(), oxygen.spo2Avg(),
  temp.objectTemp(), temp.ambientTemp());



  yield();
}