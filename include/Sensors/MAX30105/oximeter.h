/**
 * @file oximeter.h
 * @brief SpO2 (blood-oxygen saturation) calculation and state management
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 1.0
 * @details Implements state machine for SpO2 measurement with rolling window algorithm.
 *          Processes IR and Red LED samples. State flow: NoFinger → Filling → Streaming
 */

#pragma once

/*
  Oximeter -- SpO2 (blood-oxygen saturation) only.
  Heart-rate (BPM) detection lives in heartbeat.h / heartbeat.cpp.

  Design
  ------
  This class is sensor-agnostic at runtime: samples arrive via process()
  calls from main.cpp, which owns the FIFO drain loop.
  The sensor is configured once inside begin() and never touched again.

  State machine
  -------------
    NoFinger  -> IR below FINGER_THRESHOLD; waiting for finger
    Filling   -> collecting the initial BUFFER_LENGTH-sample window
    Streaming -> rolling window update + live SpO2 output

  Typical use
  -----------
    // setup()
    oxygen.begin(sensor);          // sensor configured here

    // loop() -- called once per FIFO sample drained by main.cpp
    oxygen.process(irValue, redValue);
*/

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
