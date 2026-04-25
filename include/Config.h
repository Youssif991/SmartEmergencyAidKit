#pragma once

#include <Arduino.h>
#include <Wire.h>

// ── I2C Pins (ESP32-C3) ───────────────────────────────────────────────────────
constexpr uint8_t  I2C_SDA_PIN = 8;
constexpr uint8_t  I2C_SCL_PIN = 9;

// ── Timing ────────────────────────────────────────────────────────────────────
constexpr uint32_t TEMP_INTERVAL_MS  = 2000;
constexpr uint32_t PRINT_INTERVAL_MS = 1000;

// ── MAX30105 Sensor Configuration ─────────────────────────────────────────────
// Values match the SparkFun SPO2 reference sketch (Example8_SPO2).
constexpr byte MAX30105_LED_BRIGHTNESS = 60;   // 0-255 -> 0-50 mA
constexpr byte MAX30105_AVERAGE        = 4;    // sample averaging (1/2/4/8/16/32)
constexpr byte MAX30105_LED_MODE       = 2;    // 1=Red, 2=Red+IR, 3=Red+IR+Green
constexpr int  MAX30105_RATE_HZ        = 100;  // samples/s (50/100/200/400...)
constexpr int  MAX30105_PULSE_US       = 411;  // pulse width us (69/118/215/411)
constexpr int  MAX30105_ADC_RANGE      = 4096; // ADC range (2048/4096/8192/16384)

// ── Algorithm / Buffer Settings ───────────────────────────────────────────────
constexpr uint32_t FINGER_THRESHOLD = 50000;  // IR value below which = no finger
constexpr int32_t  BUFFER_LENGTH    = 100;    // 4-second window at 25 sps
constexpr int32_t  BUFFER_SHIFT     = 25;     // samples dropped per rolling update
