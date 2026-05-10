/**
 * @file app.cpp
 * @brief BLE GATT server implementation
 * @author Youssef Mohammed, Youssef Hisham, Mahmoud Abdel-Dayem
 * @date 2026-05-06
 * @version 3.2
 */

#include "app.h"

// ---------------------------------------------------------------------------
// Private state
// ---------------------------------------------------------------------------
static NimBLEServer*         pServer   = nullptr;
static NimBLECharacteristic* pDataChar = nullptr;
static bool                  connected = false;

// ---------------------------------------------------------------------------
// Connection callbacks — re-advertise on disconnect
// ---------------------------------------------------------------------------
class BleCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer*) override
    {
        connected = true;
        Serial.println("[BLE] Client connected.");
    }

    void onDisconnect(NimBLEServer* srv) override
    {
        connected = false;
        Serial.println("[BLE] Client disconnected — restarting advertising.");
        srv->startAdvertising();
    }
};

// ---------------------------------------------------------------------------
// bleInit
// ---------------------------------------------------------------------------
void bleInit()
{
    NimBLEDevice::init(BLE_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new BleCallbacks());

    NimBLEService* svc = pServer->createService(BLE_SERVICE_UUID);
    pDataChar = svc->createCharacteristic(
        BLE_DATA_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    svc->start();

    NimBLEDevice::getAdvertising()->addServiceUUID(BLE_SERVICE_UUID);
    NimBLEDevice::getAdvertising()->start();

    Serial.println("[BLE] Advertising as \"" BLE_DEVICE_NAME "\"");
}

// ---------------------------------------------------------------------------
// sendBLE
// ---------------------------------------------------------------------------
void sendBLE(float accx, float accy, float accz,
             float gyrx, float gyry, float gyrz,
             int   hr,   int   spo2,
             float temp_obj, float temp_amb)
{
    if (!connected) return;

    char buf[256];
    snprintf(buf, sizeof(buf),
        "{"
        "\"hr\":%d,\"spo2\":%d,\"fp\":%d,"
        "\"tObj\":%.1f,\"tAmb\":%.1f,"
        "\"ax\":%.2f,\"ay\":%.2f,\"az\":%.2f,"
        "\"gx\":%.1f,\"gy\":%.1f,\"gz\":%.1f"
        "}",
        hr, spo2,
        (spo2 > 50) ? 1 : 0,
        temp_obj, temp_amb,
        accx, accy, accz,
        gyrx, gyry, gyrz
    );

    pDataChar->setValue(buf);
    pDataChar->notify();
    Serial.println(buf);
    Serial.println("[BLE] Sent.");
    delay(1000);
}