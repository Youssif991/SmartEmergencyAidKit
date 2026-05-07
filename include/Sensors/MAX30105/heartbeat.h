/**
 * @file heartbeat.h
 * @brief Heart rate detection using FFT (Fast Fourier Transform) algorithm
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 3.0
 * @details Collects IR samples into a sliding circular buffer and applies a
 *          Cooley-Tukey radix-2 FFT to find the dominant frequency in the
 *          heart-rate band (HR_MIN_BPM - HR_MAX_BPM).  Parabolic interpolation
 *          refines the peak estimate to sub-bin accuracy.
 *
 *          v3.0 additions:
 *            • Bandpass IIR filter (0.5–4 Hz) applied per-sample before the
 *              FFT buffer to strip DC drift and high-frequency noise.
 *            • SNR quality gate — result is discarded when peak/noise < threshold.
 *            • Exponential Moving Average (EMA) on the BPM output to prevent
 *              erratic display jumps between updates.
 *            • FFT window doubled to 512 samples, halving frequency resolution
 *              (≈11.7 BPM/bin) and improving parabolic interpolation accuracy.
 *
 *          The public API (beatAvg / bpm / fingerPresent / irValue) is unchanged
 *          so no other files need modification.
 */

#pragma once

#include <Arduino.h>

// ── FFT Parameters ─────────────────────────────────────────────────────────────
// HR_FFT_SAMPLES must be a power of 2.
// At HR_SAMPLE_RATE = 100 Hz → 512 samples = 5.12 s window.
// Frequency resolution = HR_SAMPLE_RATE / HR_FFT_SAMPLES = 0.195 Hz ≈ 11.7 BPM/bin.
// Parabolic interpolation refines this to < 3 BPM accuracy in practice.
constexpr uint16_t HR_FFT_SAMPLES  = 512;
constexpr float    HR_SAMPLE_RATE  = 100.0f;  // Hz — must match MAX30105_RATE_HZ
constexpr uint16_t HR_FFT_UPDATE   = 64;      // Recompute every N new samples
                                               // (= 0.64 s sliding step at 100 Hz)

// ── Detection Range ────────────────────────────────────────────────────────────
constexpr float HR_MIN_BPM = 40.0f;
constexpr float HR_MAX_BPM = 200.0f;

// ── Finger Detection Threshold ─────────────────────────────────────────────────
constexpr long HR_FINGER_THRESHOLD = 50000;

// ── Bandpass IIR Filter Coefficients ──────────────────────────────────────────
// Cascaded 1st-order high-pass (0.5 Hz) + 1st-order low-pass (4 Hz) at 100 Hz.
//
// High-pass:  α_hp = RC/(RC+dt),  RC = 1/(2π·0.5) = 0.31831 s,  dt = 0.01 s
//             α_hp = 0.31831 / (0.31831 + 0.01) = 0.96952
//
// Low-pass:   α_lp = dt/(RC+dt),  RC = 1/(2π·4.0) = 0.03979 s,  dt = 0.01 s
//             α_lp = 0.01 / (0.03979 + 0.01) = 0.20083
constexpr float HR_IIR_HP_ALPHA = 0.96952f;  // high-pass pole (removes DC / <0.5 Hz)
constexpr float HR_IIR_LP_ALPHA = 0.20083f;  // low-pass pole (removes >4 Hz noise)

// ── SNR Quality Gate ───────────────────────────────────────────────────────────
// Minimum ratio of peak-bin magnitude to average noise-floor magnitude.
// Results that fall below this threshold are discarded as unreliable.
constexpr float HR_SNR_MIN_RATIO = 2.5f;

// ── Median Filter ──────────────────────────────────────────────────────────────
// The last HR_MEDIAN_SIZE valid FFT BPM readings are stored in a ring buffer.
// Their median is computed and fed into the EMA, making the output immune to
// bin-hopping outliers (even if 2 of 5 readings land on the wrong bin, the
// median still selects the correct one).
constexpr uint8_t HR_MEDIAN_SIZE = 5;

// ── EMA Smoothing ──────────────────────────────────────────────────────────────
// Exponential moving average weight applied to each new FFT BPM estimate.
// Lower α → smoother (slower to track changes); higher α → more responsive.
// 0.15 gives good stability at the ~0.64 s update rate on top of the median filter.
constexpr float HR_EMA_ALPHA = 0.15f;

class HeartBeat {
public:
    // ── Lifecycle ──────────────────────────────────────────────────────────────

    // Call once from setup() after Serial and Wire are ready.
    // Sensor hardware is NOT touched here — the caller owns the sensor.
    void begin();

    // Feed one IR sample (call for every sample drained from the sensor FIFO).
    // The sample is bandpass-filtered before being stored in the circular buffer.
    // An FFT is triggered whenever HR_FFT_UPDATE new samples have arrived and
    // the buffer holds at least HR_FFT_SAMPLES valid entries.
    void process(long irValue);

    // ── Results ────────────────────────────────────────────────────────────────

    // EMA-smoothed BPM derived from the dominant FFT frequency.
    // Returns 0 until at least HR_FFT_SAMPLES samples have been collected
    // and at least one FFT result passes the SNR quality gate.
    int   beatAvg()       const { return (int)roundf(_bpmEMA);  }

    // Same value as beatAvg(), returned as float for API compatibility.
    float bpm()           const { return _bpmEMA;               }

    // True when the IR signal indicates a finger is present.
    bool  fingerPresent() const { return _fingerPresent;        }

    // Raw IR value from the most recent process() call (useful for diagnostics).
    long  irValue()       const { return _irValue;              }

private:
    // ── Circular sample buffer ────────────────────────────────────────────────
    float    _circBuf[HR_FFT_SAMPLES] = {};  // ring buffer of bandpass-filtered samples
    uint16_t _circHead  = 0;                 // index of the next write position
    uint32_t _totalSamp = 0;                 // total samples since begin()/reset()

    // ── FFT working arrays (reused every computation) ─────────────────────────
    float _vReal[HR_FFT_SAMPLES] = {};  // real part (receives windowed signal)
    float _vImag[HR_FFT_SAMPLES] = {};  // imaginary part (zeroed before each FFT)

    // ── Bandpass IIR filter state ─────────────────────────────────────────────
    float _hp_xPrev = 0.0f;  // high-pass: previous raw input sample
    float _hp_yPrev = 0.0f;  // high-pass: previous filtered output
    float _lp_yPrev = 0.0f;  // low-pass:  previous filtered output

    // ── Median filter ring buffer ─────────────────────────────────────────────
    float   _medianBuf[HR_MEDIAN_SIZE] = {};  // last N valid FFT BPM readings
    uint8_t _medianIdx   = 0;                 // next write position (ring)
    uint8_t _medianCount = 0;                 // readings collected (saturates at HR_MEDIAN_SIZE)

    // ── Output ────────────────────────────────────────────────────────────────
    float _bpmEMA       = 0.0f;  // EMA-smoothed BPM (float for precision)
    bool  _fingerPresent = false;
    long  _irValue       = 0;

    // ── Internal helpers ──────────────────────────────────────────────────────
    void computeFFT();  // run FFT on current buffer, apply SNR gate + EMA
    void reset();       // clear all state (finger removal / begin)
};
