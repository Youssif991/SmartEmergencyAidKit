#include "Config.h"
#include "MAX30105.h"
#include "temperature.h"
#include "oximeter.h"
#include "heartbeat.h"
#include "imu.h"
#include "oled.h"

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
void setup() {
  Serial.begin(115200);
  delay(10000); 

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  // 1. Start with the IMU at a conservative speed
  Wire.setClock(100000); 
  delay(100);
  if (!imu.begin()) {
     Serial.println("IMU failed at 100kHz, retrying at 400kHz...");
     Wire.setClock(400000);
     if(!imu.begin()) while(1); 
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
  if (!display.begin()) {
    Serial.println("[OLED] Failed to initialize display");
  } else {
    Serial.println("[OLED] Display initialized");
    display.printText("SmartAidKit", 30, 10);
    display.display();
    delay(1000);
  }
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
    Serial.printf("filling (%d/%d)\n", oxygen.fillProgress(), BUFFER_LENGTH);
    break;
  case OximeterState::Streaming:
    if (oxygen.spo2Avg() > 50)
      Serial.printf("%d%%\n", oxygen.spo2Avg());
    else
      Serial.println("calculating...");
   

  // ── Update OLED display (less frequently to save cycles) ────────────────────
  if (millis() - lastOledMs >= OLED_INTERVAL_MS)
  {
    lastOledMs = millis();
    int bpm_val = heartbeat.beatAvg() > 0 ? heartbeat.beatAvg() : 0;
    int spo2_val = oxygen.spo2Avg() > 50 ? oxygen.spo2Avg() : 0;
    display.printSensorData(temp.objectTemp(), temp.ambientTemp(),
                           bpm_val, spo2_val,
                           imu.accelX(), imu.accelY(), imu.accelZ());
  } break;
  }

  // IMU output
  if (imu.ready())
  {
    Serial.printf("IMU: Accel(g) X=%.2f Y=%.2f Z=%.2f  |  "
                  "Gyro(dps) X=%.1f Y=%.1f Z=%.1f\n",
                  imu.accelX(), imu.accelY(), imu.accelZ(),
                  imu.gyroX(),  imu.gyroY(),  imu.gyroZ());
  }

  yield();
}