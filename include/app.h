#pragma once
 
#include <NimBLEDevice.h>
#include <Arduino.h>
// ---------------------------------------------------------------------------
// BLE configuration
// ---------------------------------------------------------------------------
#define BLE_DEVICE_NAME    "SmartAidKit"
#define BLE_SERVICE_UUID   "12345678-1234-1234-1234-123456789abc"
#define BLE_DATA_CHAR_UUID "12345678-1234-1234-1234-123456789ab1"
#define BLE_NOTIFY_INTERVAL_MS 1000
 
//const String SERVICE_UUID   = "12345678-1234-1234-1234-123456789abc";
//const String DATA_CHAR_UUID = "12345678-1234-1234-1234-123456789ab1";

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------
 
// Call once in setup() — starts the GATT server and begins advertising
void bleInit();
 
// Send all sensor values to the connected Flutter app.
// Call it exactly like you called sendToFirebase().
void sendBLE(float accx, float accy, float accz,
             float gyrx, float gyry, float gyrz,
             int   hr,   int   spo2,
             float temp_obj, float temp_amb);
