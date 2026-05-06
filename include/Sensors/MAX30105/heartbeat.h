/**
 * @file heartbeat.h
 * @brief Heart rate detection using Peripheral Beat Amplitude (PBA) algorithm
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 1.0
 * @details Implements PBA algorithm for beat detection. Maintains rolling average of recent BPM values.
 *          Based on SparkFun Example5_HeartRate by Nathan Seidle
 */

#pragma once

/*
  HeartBeat — Peripheral Beat Amplitude (PBA) algorithm
  Based on SparkFun Example5_HeartRate by Nathan Seidle, Oct 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This class is intentionally sensor-agnostic.
  Feed it one IR sample per call to process(); read results with the
  getter functions below.

  Typical use (in main.cpp loop):
      sensor.check();
      while (sensor.available()) {
          heartbeat.process(sensor.getFIFOIR());
          sensor.nextSample();
      }
*/

#include <Arduino.h>
#include "heartRate.h"          // checkForBeat()

// Number of recent BPM readings averaged together (4 is good per SparkFun).
constexpr byte HR_RATE_SIZE = 4;

// IR threshold below which no finger is considered present.
constexpr long HR_FINGER_THRESHOLD = 50000;

class HeartBeat {
public:
    // ── Lifecycle ─────────────────────────────────────────────────────────────

    // Call once from setup() after Serial and Wire are ready.
    // Sensor hardware is NOT touched here — the caller owns the sensor.
    void begin();

    // Feed one IR sample (call for every sample drained from the sensor FIFO).
    // Internally runs the PBA beat-detection algorithm.
    void process(long irValue);

    // ── Results ───────────────────────────────────────────────────────────────

    // Rolling average BPM across the last HR_RATE_SIZE beats.
    // Returns 0 while fewer than HR_RATE_SIZE beats have been detected.
    int   beatAvg()       const { return _beatAvg;        }

    // Most recent instantaneous BPM (may be noisy; prefer beatAvg()).
    float bpm()           const { return _beatsPerMinute; }

    // True when IR value indicates a finger is present.
    bool  fingerPresent() const { return _fingerPresent;  }

    // Raw IR value from the last process() call (useful for diagnostics).
    long  irValue()       const { return _irValue;        }

private:
    // ── PBA state ─────────────────────────────────────────────────────────────
    byte  _rates[HR_RATE_SIZE] = {};    // circular buffer of recent BPMs
    byte  _rateSpot            = 0;
    long  _lastBeat            = 0;     // millis() timestamp of previous beat
    float _beatsPerMinute      = 0.0f;
    int   _beatAvg             = 0;

    // ── Finger tracking ───────────────────────────────────────────────────────
    bool  _fingerPresent = false;
    long  _irValue       = 0;

    // Reset all algorithm state (called when finger is removed).
    void reset();
};
