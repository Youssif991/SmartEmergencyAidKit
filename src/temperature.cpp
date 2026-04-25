#include "temperature.h"

bool TempSensor::begin() {
    // Wire must already be started by the caller (main setup).
    // The MLX90614 needs the bus at 50 kHz — caller is expected to set that
    // before calling begin(), and restore speed afterwards.
    if (!_mlx.begin(0x5A, &Wire)) {
        Serial.println("[TempSensor] MLX90614 not found — check wiring.");
        return false;
    }
    Serial.println("[TempSensor] MLX90614 OK.");
    return true;
}

void TempSensor::update() {
    if (millis() - _lastReadMs < TEMP_INTERVAL_MS) return;
    _lastReadMs = millis();

    // Slow the bus down for the MLX, read, then restore fast speed for MAX30105.
    Wire.setClock(50000);
    const double obj = _mlx.readObjectTempC();
    const double amb = _mlx.readAmbientTempC();
    Wire.setClock(400000);

    if (!isnan(obj)) _objectTemp  = (float)obj;
    if (!isnan(amb)) _ambientTemp = (float)amb;
}
