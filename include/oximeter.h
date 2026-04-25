#pragma once

#include "Config.h"         // must be first — defines BUFFER_LENGTH / BUFFER_SHIFT
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

// Wraps the MAX30105 pulse-oximeter.
// Call begin() once in setup(), then update() every loop iteration.
//
// Internally runs a three-state machine:
//   NoFinger  → no finger on the sensor
//   Filling   → collecting the initial 100-sample buffer
//   Streaming → rolling window + live beat detection

enum class OximeterState { NoFinger, Filling, Streaming };

class Oximeter {
public:
    bool begin();

    // Should be called every loop(). Drains the sensor FIFO and advances state.
    void update();

    OximeterState state()        const { return _state;     }
    int           beatAvg()      const { return _beatAvg;   }
    int           spo2Avg()      const { return _spo2Avg;   }
    int           fillProgress() const { return _fillCount; }  // 0 – BUFFER_LENGTH

private:
    // ── Helpers ──────────────────────────────────────────────────────────────
    void  collectInitialSamples();
    void  runAlgorithm();
    void  shiftBuffer();
    void  handleBeat(uint32_t irVal);
    void  handleRollingUpdate(uint32_t irVal, uint32_t redVal);
    void  reset();
    static float ema(float current, float next);

    // ── Hardware ─────────────────────────────────────────────────────────────
    MAX30105 _sensor;

    // ── Buffers ───────────────────────────────────────────────────────────────
    uint32_t _irBuffer[BUFFER_LENGTH]  = {};
    uint32_t _redBuffer[BUFFER_LENGTH] = {};

    // ── Algorithm results ────────────────────────────────────────────────────
    int32_t _spo2          = 0;
    int8_t  _validSPO2     = 0;
    int32_t _heartRate     = 0;
    int8_t  _validHeartRate = 0;

    // ── State ────────────────────────────────────────────────────────────────
    OximeterState _state     = OximeterState::NoFinger;
    int           _fillCount = 0;
    int           _rollCount = 0;
    int           _beatAvg   = 0;
    int           _spo2Avg   = 0;
    long          _lastBeat  = 0;

    static constexpr int ROLL_OFFSET = BUFFER_LENGTH - BUFFER_SHIFT;  // 75
};