#include "oximeter.h"

// ── Public ────────────────────────────────────────────────────────────────────

bool Oximeter::begin() {
    // Wire must already be started at 400 kHz by the caller.
    if (!_sensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("[Oximeter] MAX30105 not found — check wiring.");
        return false;
    }
    Serial.println("[Oximeter] MAX30105 OK.");

    _sensor.setup(
        MAX30105_SETUP_POWER,
        MAX30105_AVERAGE,
        MAX30105_LED_MODE,
        MAX30105_RATE_HZ,
        MAX30105_PULSE_US,
        MAX30105_ADC_RANGE
    );
    _sensor.setPulseAmplitudeRed(MAX30105_LED_AMP);
    _sensor.setPulseAmplitudeIR(MAX30105_LED_AMP);

    Serial.println("[Oximeter] Collecting initial samples...");
    collectInitialSamples();
    runAlgorithm();

    return true;
}

void Oximeter::update() {
    _sensor.check();
    if (!_sensor.available()) return;

    const uint32_t irVal  = _sensor.getIR();
    const uint32_t redVal = _sensor.getRed();
    _sensor.nextSample();

    if (irVal < FINGER_THRESHOLD) {
        if (_state != OximeterState::NoFinger) {
            Serial.println("[Oximeter] Finger removed — paused.");
            reset();
        }
        return;
    }

    // ── Finger present ────────────────────────────────────────────────────────
    if (_state == OximeterState::NoFinger) {
        Serial.println("[Oximeter] Finger detected. Filling buffer...");
        _lastBeat = millis();   // seed so first beat delta is meaningful
        _state    = OximeterState::Filling;
    }

    if (_state == OximeterState::Filling) {
        _redBuffer[_fillCount] = redVal;
        _irBuffer[_fillCount]  = irVal;

        if (++_fillCount >= BUFFER_LENGTH) {
            runAlgorithm();
            _rollCount = 0;
            _state     = OximeterState::Streaming;
            Serial.println("[Oximeter] Buffer ready. Streaming...");
        }

    } else {
        handleBeat(irVal);
        handleRollingUpdate(irVal, redVal);
    }
}

// ── Private ───────────────────────────────────────────────────────────────────

void Oximeter::handleBeat(uint32_t irVal) {
    if (!checkForBeat(irVal)) return;

    const long  delta      = millis() - _lastBeat;
    _lastBeat              = millis();
    const float instantBPM = 60000.0f / (float)delta;

    if (instantBPM > 20.0f && instantBPM < 200.0f) {
        _beatAvg = (int)ema((float)_beatAvg, instantBPM);
    }
}

void Oximeter::handleRollingUpdate(uint32_t irVal, uint32_t redVal) {
    _redBuffer[ROLL_OFFSET + _rollCount] = redVal;
    _irBuffer[ROLL_OFFSET  + _rollCount] = irVal;

    if (++_rollCount < BUFFER_SHIFT) return;

    shiftBuffer();
    runAlgorithm();

    if (_validSPO2 && _spo2 > 50 && _spo2 <= 100) {
        _spo2Avg = (_spo2Avg == 0) ? _spo2
                                   : (int)ema((float)_spo2Avg, (float)_spo2);
    }

    _rollCount = 0;
}

void Oximeter::collectInitialSamples() {
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        while (!_sensor.available()) _sensor.check();
        _redBuffer[i] = _sensor.getRed();
        _irBuffer[i]  = _sensor.getIR();
        _sensor.nextSample();
    }
}

void Oximeter::runAlgorithm() {
    maxim_heart_rate_and_oxygen_saturation(
        _irBuffer, BUFFER_LENGTH, _redBuffer,
        &_spo2, &_validSPO2, &_heartRate, &_validHeartRate);
}

void Oximeter::shiftBuffer() {
    for (int i = 0; i < ROLL_OFFSET; i++) {
        _redBuffer[i] = _redBuffer[i + BUFFER_SHIFT];
        _irBuffer[i]  = _irBuffer[i + BUFFER_SHIFT];
    }
}

void Oximeter::reset() {
    _beatAvg   = 0;
    _spo2Avg   = 0;
    _fillCount = 0;
    _rollCount = 0;
    _state     = OximeterState::NoFinger;
}

float Oximeter::ema(float current, float next) {
    constexpr float alpha = 0.3f;
    return (current == 0.0f) ? next
                             : (alpha * next) + ((1.0f - alpha) * current);
}
