#include "Config.h"
#include "temperature.h"
#include "oximeter.h"

TempSensor temp;
Oximeter   oximeter;

static uint32_t lastPrintMs = 0;

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("\n--- Sensor Initialization ---");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 50000);
    temp.begin();

    Wire.setClock(400000);
    if (!oximeter.begin()) {
        Serial.println("Halting — fix MAX30105 wiring.");
        while (true) delay(10);
    }

    Serial.println("System ready.\n");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
    temp.update();
    oximeter.update();

    if (millis() - lastPrintMs < PRINT_INTERVAL_MS) {
        yield();
        return;
    }
    lastPrintMs = millis();

    // ── Temperature ───────────────────────────────────────────────────────────
    Serial.printf("Temp (obj): %.1f C  |  Temp (amb): %.1f C  |  HR: ",
                  temp.objectTemp(), temp.ambientTemp());

    // ── HR / SpO2 ─────────────────────────────────────────────────────────────
    switch (oximeter.state()) {
        case OximeterState::NoFinger:
            Serial.println("-- (no finger)  |  SpO2: --");
            break;

        case OximeterState::Filling:
            Serial.printf("filling (%d/%d)  |  SpO2: filling\n",
                          oximeter.fillProgress(), BUFFER_LENGTH);
            break;

        case OximeterState::Streaming:
            if (oximeter.beatAvg() > 20) Serial.printf("%d bpm", oximeter.beatAvg());
            else                         Serial.print("Calculating...");

            Serial.print("  |  SpO2: ");
            if (oximeter.spo2Avg() > 50) Serial.printf("%d %%\n", oximeter.spo2Avg());
            else                         Serial.println("Calculating...");
            break;
    }

    yield();
}