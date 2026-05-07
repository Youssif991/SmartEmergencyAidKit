/**
 * @file heartbeat.h
 * @brief Heart rate detection using FFT (Fast Fourier Transform) algorithm
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 2.0
 * @details Collects IR samples into a sliding circular buffer and applies a
 *          Cooley-Tukey radix-2 FFT to find the dominant frequency in the
 *          heart-rate band (HR_MIN_BPM - HR_MAX_BPM).  Parabolic interpolation
 *          refines the peak estimate to sub-bin accuracy.
 *
 *          Replaces the earlier PBA beat-detection + rolling-average approach.
 *          The public API (beatAvg / bpm / fingerPresent / irValue) is unchanged
 *          so no other files need modification.
 */

#pragma once

#include <Arduino.h>

// ── FFT Parameters ─────────────────────────────────────────────────────────────
// HR_FFT_SAMPLES must be a power of 2.
// At HR_SAMPLE_RATE = 100 Hz → 256 samples = 2.56 s window.
// Frequency resolution = HR_SAMPLE_RATE / HR_FFT_SAMPLES = 0.39 Hz ≈ 23.4 BPM/bin.
// Parabolic interpolation refines this to < 5 BPM accuracy in practice.
constexpr uint16_t HR_FFT_SAMPLES  = 256;
constexpr float    HR_SAMPLE_RATE  = 100.0f;  // Hz — must match MAX30105_RATE_HZ
constexpr uint16_t HR_FFT_UPDATE   = 64;      // Recompute every N new samples
                                               // (= 0.64 s sliding step at 100 Hz)

// ── Detection Range ────────────────────────────────────────────────────────────
constexpr float HR_MIN_BPM = 40.0f;
constexpr float HR_MAX_BPM = 200.0f;

// ── Finger Detection Threshold ─────────────────────────────────────────────────
constexpr long HR_FINGER_THRESHOLD = 50000;

class HeartBeat {
public:
    // ── Lifecycle ──────────────────────────────────────────────────────────────

    // Call once from setup() after Serial and Wire are ready.
    // Sensor hardware is NOT touched here — the caller owns the sensor.
    void begin();

    // Feed one IR sample (call for every sample drained from the sensor FIFO).
    // Internally accumulates samples and triggers an FFT when the window is full.
    void process(long irValue);

    // ── Results ────────────────────────────────────────────────────────────────

    // BPM derived from the dominant FFT frequency.
    // Returns 0 until at least HR_FFT_SAMPLES samples have been collected.
    int   beatAvg()       const { return _bpmFFT;        }

    // Same value as beatAvg(), returned as float for API compatibility.
    float bpm()           const { return (float)_bpmFFT; }

    // True when the IR signal indicates a finger is present.
    bool  fingerPresent() const { return _fingerPresent;  }

    // Raw IR value from the most recent process() call (useful for diagnostics).
    long  irValue()       const { return _irValue;        }

private:
    // ── Circular sample buffer ────────────────────────────────────────────────
    float    _circBuf[HR_FFT_SAMPLES] = {};  // ring buffer of raw IR samples
    uint16_t _circHead  = 0;                 // index of the next write position
    uint32_t _totalSamp = 0;                 // total samples since begin()/reset()

    // ── FFT working arrays (reused every computation) ─────────────────────────
    float _vReal[HR_FFT_SAMPLES] = {};  // real part (receives windowed signal)
    float _vImag[HR_FFT_SAMPLES] = {};  // imaginary part (zeroed before each FFT)

    // ── Output ────────────────────────────────────────────────────────────────
    int  _bpmFFT        = 0;
    bool _fingerPresent = false;
    long _irValue       = 0;

    // ── Internal helpers ──────────────────────────────────────────────────────
    void computeFFT();  // run FFT on current buffer, update _bpmFFT
    void reset();       // clear all state (finger removal / begin)
};
