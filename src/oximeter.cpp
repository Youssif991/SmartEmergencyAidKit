/*
  oximeter.cpp -- SpO2 (blood-oxygen saturation) computation.

  Sensor configuration
  --------------------
  begin() owns all MAX30105 initialisation so main.cpp stays clean.
  Settings are taken directly from the SparkFun Example8_SPO2 reference sketch.

  Sample flow
  -----------
  main.cpp drains the sensor FIFO each loop() tick and calls:
      oxygen.process(irValue, redValue);
  for every new sample.  This class never touches the sensor after begin().
*/

#include "oximeter.h"
#include <Arduino.h>

// ── Public ────────────────────────────────────────────────────────────────────

bool Oximeter::begin(MAX30105 &sensor)
{
    // -- Sensor hardware init (settings mirror Example8_SPO2) -----------------
    if (!sensor.begin(Wire, I2C_SPEED_FAST))
    {
        Serial.println("[Oximeter] MAX30105 not found -- check wiring.");
        return false;
    }
    Serial.println("[Oximeter] MAX30105 found.");

    sensor.setup(
        MAX30105_LED_BRIGHTNESS, // LED power   (60 ~ 12 mA)
        MAX30105_AVERAGE,        // averaging   (4 samples)
        MAX30105_LED_MODE,       // LED mode    (Red + IR)
        MAX30105_RATE_HZ,        // sample rate (100 Hz)
        MAX30105_PULSE_US,       // pulse width (411 us)
        MAX30105_ADC_RANGE       // ADC range   (4096)
    );
    sensor.setPulseAmplitudeGreen(0); // green LED not needed in mode 2

    // -- Software state reset --------------------------------------------------
    reset();
    Serial.println("[Oximeter] Ready -- SpO2 module initialised.");
    return true;
}

void Oximeter::process(uint32_t irVal, uint32_t redVal)
{
    // ── Finger detection ──────────────────────────────────────────────────────
    if (irVal < FINGER_THRESHOLD)
    {
        if (_state != OximeterState::NoFinger)
        {
            Serial.println("[Oximeter] Finger removed -- SpO2 paused.");
            reset();
        }
        return;
    }

    // ── Finger present ────────────────────────────────────────────────────────
    if (_state == OximeterState::NoFinger)
    {
        Serial.println("[Oximeter] Finger detected. Filling SpO2 buffer...");
        _state = OximeterState::Filling;
    }

    // ── Filling: build the initial 100-sample window ──────────────────────────
    if (_state == OximeterState::Filling)
    {
        _redBuffer[_fillCount] = redVal;
        _irBuffer[_fillCount] = irVal;

        if (++_fillCount >= BUFFER_LENGTH)
        {
            runAlgorithm();
            _rollCount = 0;
            _state = OximeterState::Streaming;
            Serial.println("[Oximeter] Buffer full. Streaming SpO2...");
        }
        return;
    }

    // ── Streaming: rolling window (shift 25, collect 25, recalculate) ─────────
    handleRollingUpdate(irVal, redVal);
}

// ── Private ───────────────────────────────────────────────────────────────────

void Oximeter::handleRollingUpdate(uint32_t irVal, uint32_t redVal)
{
    _redBuffer[ROLL_OFFSET + _rollCount] = redVal;
    _irBuffer[ROLL_OFFSET + _rollCount] = irVal;

    if (++_rollCount < BUFFER_SHIFT)
        return; // wait for 25 new samples

    shiftBuffer();
    runAlgorithm();

    if (_validSPO2 && _spo2 > 50 && _spo2 <= 100)
    {
        _spo2Avg = (_spo2Avg == 0) ? (int)_spo2
                                   : (int)ema((float)_spo2Avg, (float)_spo2);
    }

    _rollCount = 0;
}

void Oximeter::runAlgorithm()
{
    maxim_heart_rate_and_oxygen_saturation(
        _irBuffer, BUFFER_LENGTH, _redBuffer,
        &_spo2, &_validSPO2,
        &_heartRate, &_validHeartRate);
}

void Oximeter::shiftBuffer()
{
    // Discard oldest BUFFER_SHIFT (25) samples; keep the newest 75.
    for (int i = 0; i < ROLL_OFFSET; i++)
    {
        _redBuffer[i] = _redBuffer[i + BUFFER_SHIFT];
        _irBuffer[i] = _irBuffer[i + BUFFER_SHIFT];
    }
}

void Oximeter::reset()
{
    // Clear all buffers to prevent old data from interfering with new calculations
    memset(_irBuffer, 0, sizeof(_irBuffer));
    memset(_redBuffer, 0, sizeof(_redBuffer));
    
    _spo2 = 0;
    _validSPO2 = 0;
    _heartRate = 0;
    _validHeartRate = 0;
    _spo2Avg = 0;
    _fillCount = 0;
    _rollCount = 0;
    _state = OximeterState::NoFinger;
}

float Oximeter::ema(float current, float next)
{
    constexpr float alpha = 0.3f;
    return (current == 0.0f) ? next
                             : (alpha * next) + ((1.0f - alpha) * current);
}
