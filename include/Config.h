#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  config.h  —  Central configuration for all includes and shared variables
// ─────────────────────────────────────────────────────────────────────────────

// ── Core Arduino / ESP32 ─────────────────────────────────────────────────────
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

// ── Sensors ───────────────────────────────────────────────────────────────────
#include "MAX30100_PulseOximeter.h"
#include "MLX90614.h"
#include "MAX30100_SpO2Calculator.h"
#include "MAX30100_BeatDetector.h"
#include "CircularBuffer.h"
#include "Crc8.h"
#include "MAX30100.h"

// ═════════════════════════════════════════════════════════════════════════════
//  Timing intervals (ms)
// ═════════════════════════════════════════════════════════════════════════════
#define REPORTING_PERIOD_MS 1000   // Print readings every second
#define TEMP_PERIOD_MS      5000   // MLX read every 5 s (slow sensor)

// ═════════════════════════════════════════════════════════════════════════════

// Sensors
extern PulseOximeter  pox;
extern MLX90614       mlx;

// Timers
extern uint32_t tsLastPush;
extern uint32_t tsLastTemp;

// Latest readings
extern float    tempC;
extern float    hr;
extern uint8_t  spo2;