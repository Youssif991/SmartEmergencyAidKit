#include "Config.h"
#include "MAX30105.h"
#include "temperature.h"
#include "oximeter.h"
#include "heartbeat.h"

// ── Module instances ──────────────────────────────────────────────────────────
MAX30105 sensor;
TempSensor temp;
Oximeter oxygen;
HeartBeat heartbeat;

static uint32_t lastPrintMs = 0;

// ─────────────────────────────────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);
  delay(10000);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  temp.begin();
  if (!oxygen.begin(sensor))
    while (true)
      delay(10); // halt on wiring error
  heartbeat.begin();

  Serial.println("\n--- SmartAidKit ready ---\n");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop()
{
  temp.update();

  // Drain sensor FIFO and feed each sample to both modules.
  sensor.check();
  while (sensor.available())
  {
    const uint32_t ir = sensor.getFIFOIR();
    const uint32_t red = sensor.getFIFORed();
    sensor.nextSample();

    oxygen.process(ir, red); // SpO2  (IR + Red)
    heartbeat.process(ir);   // BPM   (IR only, PBA algorithm)
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
    break;
  }

  yield();
}
