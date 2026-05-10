/**
 * @file oximeter.h
 * @brief SpO2 (blood-oxygen saturation) calculation and state management
 * @author Youssef Mohammed, Youssef Hisham, Mahmoud Abdeldayem
 * @date 2026-05-06
 * @version 1.0
 * @details Implements state machine for SpO2 measurement with rolling window algorithm.
 *          Processes IR and Red LED samples. State flow: NoFinger → Filling → Streaming
 */

#pragma once

#include "../../Core/Config.h"    // BUFFER_LENGTH, BUFFER_SHIFT, FINGER_THRESHOLD
#include "MAX30105.h"             // only needed in begin() for sensor init
#include "spo2_algorithm.h"       // maxim_heart_rate_and_oxygen_saturation()

enum class OximeterState
{
    NoFinger,
    Filling,
    Streaming
};

class Oximeter
{
public:
    // ── Lifecycle ──────────────────────────────────────────────────────────────

    // Initialises the MAX30105 hardware with SpO2-optimal settings and resets
    // internal state. Call once from setup() after Wire.begin().
    // Returns false if the sensor is not found (wiring error).
    bool begin(MAX30105 &sensor);

    // Feed one IR + Red sample pair (call for every sample drained from the
    // sensor FIFO). Advances the internal state machine and recomputes SpO2
    // every BUFFER_SHIFT new samples.
    void process(uint32_t irValue, uint32_t redValue);

    // ── Results ────────────────────────────────────────────────────────────────

    OximeterState state() const { return _state; }
    int spo2Avg() const { return _spo2Avg; }

    // Samples collected so far during the Filling phase (0 - BUFFER_LENGTH).
    int fillProgress() const { return _fillCount; }

private:
    // ── SpO2 buffers ───────────────────────────────────────────────────────────
    uint32_t _irBuffer[BUFFER_LENGTH] = {};
    uint32_t _redBuffer[BUFFER_LENGTH] = {};

    // ── Algorithm outputs ──────────────────────────────────────────────────────
    int32_t _spo2 = 0;
    int8_t _validSPO2 = 0;
    int32_t _heartRate = 0; // produced by algorithm but not exposed here
    int8_t _validHeartRate = 0;

    // ── State ──────────────────────────────────────────────────────────────────
    OximeterState _state = OximeterState::NoFinger;
    int _fillCount = 0;
    int _rollCount = 0;
    int _spo2Avg = 0;

    static constexpr int ROLL_OFFSET = BUFFER_LENGTH - BUFFER_SHIFT; // 75

    // ── Helpers ────────────────────────────────────────────────────────────────
    void runAlgorithm();
    void shiftBuffer();
    void handleRollingUpdate(uint32_t irVal, uint32_t redVal);
    void reset();
    static float ema(float current, float next);
};
73: 
74: /** @} */
