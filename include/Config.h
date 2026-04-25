#pragma once

#include <Arduino.h>
#include <Wire.h>

// ── I2C Pins (ESP32-C3) ───────────────────────────────────────────────────────
constexpr uint8_t  I2C_SDA_PIN = 8;
constexpr uint8_t  I2C_SCL_PIN = 9;

// ── Timing ────────────────────────────────────────────────────────────────────
constexpr uint32_t TEMP_INTERVAL_MS  = 2000;
constexpr uint32_t PRINT_INTERVAL_MS = 1000;

// ── MAX30105 Sensor Configuration ────────────────────────────────────────────
constexpr byte MAX30105_SETUP_POWER = 0x1F;  // ~6.4 mA
constexpr byte MAX30105_AVERAGE     = 4;     // sample averaging
constexpr byte MAX30105_LED_MODE    = 2;     // Red + IR
constexpr int  MAX30105_RATE_HZ     = 100;   // samples/s
constexpr int  MAX30105_PULSE_US    = 411;   // pulse width
constexpr int  MAX30105_ADC_RANGE   = 4096;
constexpr byte MAX30105_LED_AMP     = 0x1F;  // LED brightness (was 0x0A — too dim)

// ── Algorithm / Buffer Settings ──────────────────────────────────────────────
constexpr uint16_t FINGER_THRESHOLD = 5000;
constexpr int32_t  BUFFER_LENGTH    = 100;   // 4-second window at 25 Hz
constexpr int32_t  BUFFER_SHIFT     = 25;    // samples dropped per rolling update