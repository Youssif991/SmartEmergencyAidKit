#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  config.h  —  Central configuration for all includes and shared variables
// ─────────────────────────────────────────────────────────────────────────────

// ── Core Arduino / ESP32 ─────────────────────────────────────────────────────
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

// ── Firebase ─────────────────────────────────────────────────────────────────
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ── Sensors ───────────────────────────────────────────────────────────────────
#include "MAX30100_PulseOximeter.h"
#include "MLX90614.h"

// ═════════════════════════════════════════════════════════════════════════════
//  WiFi & Firebase credentials  —  edit these
// ═════════════════════════════════════════════════════════════════════════════
#define WIFI_SSID       "------"
#define WIFI_PASSWORD   "------"
#define API_KEY         "AIzaSyC-ad77Wyw4_9FCQIbQwq53y-BRY02oe_o"
#define DATABASE_URL    "https://health-71a04-default-rtdb.europe-west1.firebasedatabase.app/"

// ═════════════════════════════════════════════════════════════════════════════
//  Timing intervals (ms)
// ═════════════════════════════════════════════════════════════════════════════
#define FIREBASE_PUSH_MS  2000   // How often to push readings to Firebase
#define TEMP_READ_MS      5000   // How often to read the MLX90614

// ═════════════════════════════════════════════════════════════════════════════
//  Shared global objects  —  defined once in main.cpp, declared extern here
//  so any future .cpp file can access them with just #include "config.h"
// ═════════════════════════════════════════════════════════════════════════════

// Firebase
extern FirebaseData   fbdo;
extern FirebaseAuth   auth;
extern FirebaseConfig config;
extern bool           signupOK;

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