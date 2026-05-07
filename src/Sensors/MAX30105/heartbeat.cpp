/**
 * @file heartbeat.cpp
 * @brief Heart rate detection using FFT algorithm
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 2.0
 * @details
 *   Algorithm overview
 *   ──────────────────
 *   1. IR samples are stored in a circular buffer of HR_FFT_SAMPLES entries.
 *   2. Once the buffer is full (HR_FFT_SAMPLES received), a new FFT is run
 *      every HR_FFT_UPDATE samples — a sliding-window approach that gives an
 *      updated BPM estimate every ~0.64 s (at 100 Hz).
 *   3. Before the FFT:
 *        a. DC is removed by subtracting the window mean.
 *        b. A Hann window is applied to reduce spectral leakage.
 *   4. The magnitude spectrum is scanned in the HR_MIN_BPM – HR_MAX_BPM band.
 *   5. Parabolic interpolation refines the peak-bin estimate for sub-bin
 *      accuracy, then the frequency is converted to BPM.
 *
 *   Replaces SparkFun PBA (checkForBeat) + rolling BPM average.
 */

#include "../../../include/Sensors/MAX30105/heartbeat.h"
#include <math.h>

// ── In-place Cooley-Tukey radix-2 DIT FFT ─────────────────────────────────────
// n must be a power of 2.  Both vR (real) and vI (imaginary) are modified
// in-place.  On return, vR[k]+j*vI[k] is the k-th complex DFT coefficient.

static void fft_inplace(float *vR, float *vI, uint16_t n)
{
    // ── Bit-reversal permutation ──────────────────────────────────────────────
    uint16_t j = 0;
    for (uint16_t i = 1; i < n; i++) {
        uint16_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            float tmp;
            tmp = vR[i]; vR[i] = vR[j]; vR[j] = tmp;
            tmp = vI[i]; vI[i] = vI[j]; vI[j] = tmp;
        }
    }

    // ── Butterfly stages ─────────────────────────────────────────────────────
    for (uint16_t len = 2; len <= n; len <<= 1) {
        const float ang    = -2.0f * (float)M_PI / (float)len;
        const float wRstep = cosf(ang);
        const float wIstep = sinf(ang);

        for (uint16_t i = 0; i < n; i += len) {
            float wR = 1.0f, wI = 0.0f;
            const uint16_t half = len >> 1;
            for (uint16_t k = 0; k < half; k++) {
                const uint16_t u = i + k;
                const uint16_t v = i + k + half;
                const float tR = wR * vR[v] - wI * vI[v];
                const float tI = wR * vI[v] + wI * vR[v];
                vR[v] = vR[u] - tR;
                vI[v] = vI[u] - tI;
                vR[u] += tR;
                vI[u] += tI;
                const float nwR = wR * wRstep - wI * wIstep;
                wI  = wR * wIstep + wI * wRstep;
                wR  = nwR;
            }
        }
    }
}

// ── Public ────────────────────────────────────────────────────────────────────

void HeartBeat::begin()
{
    reset();
    Serial.println("[HeartBeat] Ready (FFT mode) — place your index finger on the sensor.");
}

void HeartBeat::process(long irValue)
{
    _irValue = irValue;

    // ── Finger detection ──────────────────────────────────────────────────────
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
    }

    // ── Store sample in circular buffer ───────────────────────────────────────
    _circBuf[_circHead] = (float)irValue;
    _circHead = (_circHead + 1) % HR_FFT_SAMPLES;
    _totalSamp++;

    // ── Trigger FFT: once full, then every HR_FFT_UPDATE samples ─────────────
    if (_totalSamp >= HR_FFT_SAMPLES && (_totalSamp % HR_FFT_UPDATE) == 0) {
        computeFFT();
    }
}

// ── Private ───────────────────────────────────────────────────────────────────

void HeartBeat::computeFFT()
{
    // 1. Copy circular buffer into _vReal in chronological order.
    //    After a wrap, _circHead is the oldest sample's position.
    for (uint16_t i = 0; i < HR_FFT_SAMPLES; i++) {
        _vReal[i] = _circBuf[(_circHead + i) % HR_FFT_SAMPLES];
        _vImag[i] = 0.0f;
    }

    // 2. Remove DC by subtracting the window mean.
    float mean = 0.0f;
    for (uint16_t i = 0; i < HR_FFT_SAMPLES; i++) mean += _vReal[i];
    mean /= (float)HR_FFT_SAMPLES;
    for (uint16_t i = 0; i < HR_FFT_SAMPLES; i++) _vReal[i] -= mean;

    // 3. Apply Hann window to suppress spectral leakage at window edges.
    for (uint16_t i = 0; i < HR_FFT_SAMPLES; i++) {
        const float w = 0.5f * (1.0f - cosf(2.0f * (float)M_PI
                                             * (float)i
                                             / (float)(HR_FFT_SAMPLES - 1)));
        _vReal[i] *= w;
    }

    // 4. In-place FFT.
    fft_inplace(_vReal, _vImag, HR_FFT_SAMPLES);

    // 5. Search for the peak magnitude bin within the HR frequency band.
    //    bin-to-frequency: f[k] = k * HR_SAMPLE_RATE / HR_FFT_SAMPLES
    //    → bin for HR_MIN_BPM = (HR_MIN_BPM / 60) * HR_FFT_SAMPLES / HR_SAMPLE_RATE
    const uint16_t kMin = (uint16_t)((HR_MIN_BPM / 60.0f)
                                      * (float)HR_FFT_SAMPLES / HR_SAMPLE_RATE);
    const uint16_t kMax = (uint16_t)((HR_MAX_BPM / 60.0f)
                                      * (float)HR_FFT_SAMPLES / HR_SAMPLE_RATE) + 1;
    const uint16_t kNyq = HR_FFT_SAMPLES / 2;  // only positive-frequency bins

    float    maxMagSq = -1.0f;
    uint16_t peakBin  = kMin;

    for (uint16_t k = kMin; k <= kMax && k < kNyq; k++) {
        const float magSq = _vReal[k] * _vReal[k] + _vImag[k] * _vImag[k];
        if (magSq > maxMagSq) {
            maxMagSq = magSq;
            peakBin  = k;
        }
    }

    // 6. Parabolic interpolation for sub-bin accuracy.
    //    Fits a parabola through (k-1, k, k+1) magnitude-squared values
    //    and finds its analytic peak.
    float refinedBin = (float)peakBin;
    if (peakBin > kMin && peakBin < (kNyq - 1)) {
        const float mL = _vReal[peakBin-1]*_vReal[peakBin-1]
                       + _vImag[peakBin-1]*_vImag[peakBin-1];
        const float mC = maxMagSq;
        const float mR = _vReal[peakBin+1]*_vReal[peakBin+1]
                       + _vImag[peakBin+1]*_vImag[peakBin+1];
        const float denom = mL - 2.0f * mC + mR;
        if (fabsf(denom) > 1e-9f) {
            refinedBin += 0.5f * (mL - mR) / denom;
        }
    }

    // 7. Convert refined bin to BPM.
    const float freqHz = refinedBin * HR_SAMPLE_RATE / (float)HR_FFT_SAMPLES;
    const float bpm    = freqHz * 60.0f;

    if (bpm >= HR_MIN_BPM && bpm <= HR_MAX_BPM) {
        _bpmFFT = (int)roundf(bpm);
        Serial.printf("[HeartBeat] FFT BPM = %d  (bin %.2f, %.3f Hz)\n",
                      _bpmFFT, refinedBin, freqHz);
    }
}

void HeartBeat::reset()
{
    memset(_circBuf, 0, sizeof(_circBuf));
    memset(_vReal,   0, sizeof(_vReal));
    memset(_vImag,   0, sizeof(_vImag));
    _circHead      = 0;
    _totalSamp     = 0;
    _bpmFFT        = 0;
    _fingerPresent = false;
    _irValue       = 0;
}
