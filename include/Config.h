#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "Property.h"
#include <Adafruit_MLX90614.h> // Changed to Adafruit library
#include "helpers.h"

// ── I2C Pins (ESP32-C3 DevKitC-02) ───────────────────────────────────────────
constexpr uint8_t  I2C_SDA_PIN = 8;
constexpr uint8_t  I2C_SCL_PIN = 9;
constexpr uint32_t I2C_CLOCK_HZ = 100000UL;  // safe shared bus speed for both sensors

// ── Algorithm / Buffer Settings ──────────────────────────────────────────────
constexpr uint16_t FINGER_THRESHOLD     = 5000;  // Minimum IR value to trigger beat detection
constexpr int32_t  BUFFER_LENGTH        = 100;    // 4-second window at 25Hz
constexpr int32_t  BUFFER_SHIFT         = 25;     // How many old samples to drop when shifting

// ── MAX30105 Sensor Configuration ────────────────────────────────────────────
constexpr byte     MAX30105_SETUP_POWER = 0x1F;   // General power level setting (0x1F = ~6.4mA)
constexpr byte     MAX30105_AVERAGE     = 4;      // Sample averaging (options: 1, 2, 4, 8, 16, 32)
constexpr byte     MAX30105_LED_MODE    = 2;      // 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
constexpr int      MAX30105_RATE_HZ     = 100;    // Samples per second (options: 50, 100, 200, 400...)
constexpr int      MAX30105_PULSE_US    = 411;    // Pulse width in us (options: 69, 118, 215, 411)
constexpr int      MAX30105_ADC_RANGE   = 4096;   // ADC Range (options: 2048, 4096, 8192, 16384)

// ── LED Pulse Amplitudes ─────────────────────────────────────────────────────
constexpr byte     MAX30105_LED_AMP     = 0x0A;   // Operating brightness for Red/IR LEDs (0x00 to 0xFF)

// ── Beat Detection Settings ──────────────────────────────────────────────────
constexpr byte     RATE_SIZE            = 4;

// ── Timing Intervals ─────────────────────────────────────────────────────────
constexpr uint32_t TEMP_INTERVAL_MS   = 2000;
constexpr uint32_t PRINT_INTERVAL_MS  = 1000;

// ── Shared Objects ───────────────────────────────────────────────────────────
extern MAX30105 particleSensor;
extern Adafruit_MLX90614 therm; // Changed to Adafruit object

// ── Shared State Variables ───────────────────────────────────────────────────
extern int hrState;
extern int fillCount;
extern int rollCount;

// Temperature
extern float    objectTemp;
extern float    ambientTemp;
extern uint32_t lastTempMs;
extern uint32_t lastPrintMs;

// SpO2 / HR Buffers
extern uint32_t irBuffer[BUFFER_LENGTH];
extern uint32_t redBuffer[BUFFER_LENGTH];

// SpO2 / HR Values
extern int32_t  spo2;
extern int8_t   validSPO2;
extern int32_t  heartRate;
extern int8_t   validHeartRate;

// Beat Detection & Averages
extern float    beatsPerMinute;
extern int      beatAvg;
extern int      spo2Avg;
extern byte     rates[RATE_SIZE];
extern byte     rateSpot;
extern long     lastBeat;