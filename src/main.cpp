#include <Arduino.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "MLX90614.h"

// ── Sensor instances ─────────────────────────────────────────────────────────
PulseOximeter pox;
MLX90614      mlx;

// ── Timing ───────────────────────────────────────────────────────────────────
#define REPORTING_PERIOD_MS 1000   // Print readings every second
#define TEMP_PERIOD_MS      5000   // MLX read every 5 s (slow sensor)

uint32_t tsLastReport = 0;
uint32_t tsLastTemp   = 0;

float   tempC = 0.0;
float   tempF = 0.0;

// ── Beat callback ─────────────────────────────────────────────────────────────
void onBeatDetected() {
  Serial.println("Beat!");
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("MAX30100 + MLX90614 Health Monitor");

  Wire.begin();

  // ── MAX30100 ───────────────────────────────────────────────────────────────
  if (!pox.begin()) {
    Serial.println("ERROR: MAX30100 not found. Check wiring.");
    while (true);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  Serial.println("MAX30100 ready.");

  // ── MLX90614 ──────────────────────────────────────────────────────────────
  if (!mlx.begin()) {
    Serial.println("ERROR: MLX90614 not found. Check wiring.");
    while (true);
  }
  Serial.println("MLX90614 ready.");
  Serial.println("-------------------------------------------");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  // Must be called as fast as possible to keep up with the sensor FIFO
  pox.update();

  uint32_t now = millis();

  // ── Read temperature periodically ─────────────────────────────────────────
  if (now - tsLastTemp >= TEMP_PERIOD_MS) {
    tsLastTemp = now;
    tempC = (float)mlx.readTemp(MLX90614::MLX90614_SRC01, MLX90614::MLX90614_TC);
    tempF = (float)mlx.readTemp(MLX90614::MLX90614_SRC01, MLX90614::MLX90614_TF);
  }

  // ── Print all readings periodically ───────────────────────────────────────
  if (now - tsLastReport >= REPORTING_PERIOD_MS) {
    tsLastReport = now;

    float    hr   = pox.getHeartRate();
    uint8_t  spo2 = pox.getSpO2();

    Serial.print("Heart Rate: ");
    if (hr > 0) {
      Serial.print(hr, 1);
      Serial.print(" bpm");
    } else {
      Serial.print("-- (no finger?)");
    }

    Serial.print("  |  SpO2: ");
    if (spo2 > 0) {
      Serial.print(spo2);
      Serial.print("%");
    } else {
      Serial.print("--");
    }

    Serial.print("  |  Temp: ");
    Serial.print(tempC, 1);
    Serial.print("C / ");
    Serial.print(tempF, 1);
    Serial.println("F");
  }
}