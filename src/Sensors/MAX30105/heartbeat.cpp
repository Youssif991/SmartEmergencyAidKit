/**
 * @file heartbeat.cpp
 * @brief Implementation of heart rate detection using PBA algorithm
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 1.0
 * @details Peripheral Beat Amplitude algorithm implementation for heart rate detection.
 *          Based on SparkFun Example5_HeartRate
 */

#include "../../../include/Sensors/MAX30105/heartbeat.h"

// ── Public ────────────────────────────────────────────────────────────────────

void HeartBeat::begin() {
    reset();
    Serial.println("[HeartBeat] Ready — place your index finger on the sensor.");
}

void HeartBeat::process(long irValue) {
    _irValue = irValue;

    // ── Finger detection ─────────────────────────────────────────────────────
    if (irValue < HR_FINGER_THRESHOLD) {
        if (_fingerPresent) {
            Serial.println("[HeartBeat] Finger removed.");
            reset();
        }
        return;
    }

    if (!_fingerPresent) {
        Serial.println("[HeartBeat] Finger detected.");
        _fingerPresent = true;
        _lastBeat      = millis();  // seed so first delta is meaningful
  //possibly add return here 
    }
        
    // ── PBA beat detection ───────────────────────────────────────────────────
    if (!checkForBeat(irValue)) return;

    const long  delta      = millis() - _lastBeat;
    _lastBeat              = millis();
   if (delta == 0) return; //divisin by zero protection
    _beatsPerMinute        = 60.0f / (delta / 1000.0f);

    // Plausibility gate: ignore readings outside human heart-rate range.
    // Normal: 60-100 bpm | Exercise: up to 200 bpm | Physically impossible: 255+ bpm
    if (_beatsPerMinute <= 200.0f && _beatsPerMinute >= 40.0f) {
        _rates[_rateSpot++] = (byte)_beatsPerMinute; // store in circular buffer
        _rateSpot %= HR_RATE_SIZE;                   // wrap index

        // Compute rolling average.
        _beatAvg = 0;
        for (byte x = 0; x < HR_RATE_SIZE; x++) {
            _beatAvg += _rates[x];
        }
        _beatAvg /= HR_RATE_SIZE;
    }
}

// ── Private ───────────────────────────────────────────────────────────────────

void HeartBeat::reset() {
    memset(_rates, 0, sizeof(_rates));
    _rateSpot       = 0;
    _lastBeat       = 0;
    _beatsPerMinute = 0.0f;
    _beatAvg        = 0;
    _fingerPresent  = false;
    _irValue        = 0;
}
