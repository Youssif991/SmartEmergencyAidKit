/**
 * @file heartbeat.cpp
 * @brief Heart rate detection using FFT algorithm
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 3.1
 * @details
 *   Algorithm overview
 *   ──────────────────
 *   1. Each incoming IR sample is passed through a cascaded bandpass IIR filter
 *      (high-pass at 0.5 Hz + low-pass at 4 Hz) to remove DC drift and noise
 *      before it enters the FFT buffer.
 *   2. Filtered samples are stored in a circular buffer of HR_FFT_SAMPLES (512)
 *      entries.  Once the buffer is full, a new FFT is run every HR_FFT_UPDATE
 *      samples — a sliding-window approach giving an updated estimate every ~0.64 s
 *      at 100 Hz.
 *   3. Before the FFT:
 *        a. DC is removed by subtracting the window mean.
 *        b. A Hann window is applied to reduce spectral leakage.
 *   4. The magnitude spectrum is scanned in the HR_MIN_BPM – HR_MAX_BPM band.
 *   5. An SNR quality gate compares the peak magnitude to the average noise floor;
 *      results that fail (peak/noise < HR_SNR_MIN_RATIO) are discarded rather than
 *      outputting a garbage BPM.
 *   6. Parabolic interpolation refines the peak-bin estimate for sub-bin accuracy,
 *      then the frequency is converted to BPM.
 *   7. Valid raw BPM readings are pushed into a 5-slot median ring buffer.
 *      The median of up to 5 recent readings is taken, making the output immune
 *      to bin-hopping outliers (2 bad reads out of 5 cannot corrupt the median).
 *   8. The median BPM is blended into an Exponential Moving Average for the
 *      final, stable displayed value.
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

    // ── Bandpass IIR filter (0.5–4 Hz) ───────────────────────────────────────
    // Stage 1 — 1st-order high-pass (removes DC drift and signals below 0.5 Hz).
    //   y_hp[n] = α_hp * (y_hp[n-1] + x[n] - x[n-1])
    const float xn     = (float)irValue;
    const float hp_out = HR_IIR_HP_ALPHA * (_hp_yPrev + xn - _hp_xPrev);
    _hp_xPrev = xn;
    _hp_yPrev = hp_out;

    // Stage 2 — 1st-order low-pass (removes muscle artefacts and signals above 4 Hz).
    //   y_lp[n] = y_lp[n-1] + α_lp * (x[n] - y_lp[n-1])
    const float lp_out = _lp_yPrev + HR_IIR_LP_ALPHA * (hp_out - _lp_yPrev);
    _lp_yPrev = lp_out;

    // ── Store bandpass-filtered sample in circular buffer ─────────────────────
    _circBuf[_circHead] = lp_out;
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

    float    maxMag   = -1.0f;
    uint16_t peakBin  = kMin;

    for (uint16_t k = kMin; k <= kMax && k < kNyq; k++) {
        const float mag = sqrtf(_vReal[k] * _vReal[k] + _vImag[k] * _vImag[k]);
        if (mag > maxMag) {
            maxMag  = mag;
            peakBin = k;
        }
    }

    // 6. SNR quality gate — compare the peak magnitude to the average noise floor
    //    across all positive-frequency bins (excluding the HR band itself so the
    //    peak doesn't inflate the floor estimate).
    float noiseSum   = 0.0f;
    uint16_t noiseCt = 0;
    for (uint16_t k = 1; k < kNyq; k++) {
        if (k < kMin || k > kMax) {   // outside HR band → pure noise
            noiseSum += sqrtf(_vReal[k] * _vReal[k] + _vImag[k] * _vImag[k]);
            noiseCt++;
        }
    }
    const float noiseFloor = (noiseCt > 0) ? (noiseSum / (float)noiseCt) : 1.0f;
    const float snr        = (noiseFloor > 0.0f) ? (maxMag / noiseFloor) : 0.0f;

    if (snr < HR_SNR_MIN_RATIO) {
        Serial.printf("[HeartBeat] FFT discarded — SNR %.2f < %.2f\n",
                      snr, HR_SNR_MIN_RATIO);
        return;  // signal too weak or noisy; keep existing EMA value
    }

    // 7. Parabolic interpolation for sub-bin accuracy.
    //    Fits a parabola through (k-1, k, k+1) magnitude values
    //    and finds its analytic peak.
    float refinedBin = (float)peakBin;
    if (peakBin > kMin && peakBin < (kNyq - 1)) {
        const float mL = sqrtf(_vReal[peakBin-1]*_vReal[peakBin-1]
                              + _vImag[peakBin-1]*_vImag[peakBin-1]);
        const float mC = maxMag;
        const float mR = sqrtf(_vReal[peakBin+1]*_vReal[peakBin+1]
                              + _vImag[peakBin+1]*_vImag[peakBin+1]);
        const float denom = mL - 2.0f * mC + mR;
        if (fabsf(denom) > 1e-9f) {
            refinedBin += 0.5f * (mL - mR) / denom;
        }
    }

    // 8. Convert refined bin to BPM.
    const float freqHz = refinedBin * HR_SAMPLE_RATE / (float)HR_FFT_SAMPLES;
    const float bpm    = freqHz * 60.0f;

    if (bpm >= HR_MIN_BPM && bpm <= HR_MAX_BPM) {
        // 9. Push raw FFT BPM into the median ring buffer.
        _medianBuf[_medianIdx] = bpm;
        _medianIdx = (_medianIdx + 1) % HR_MEDIAN_SIZE;
        if (_medianCount < HR_MEDIAN_SIZE) _medianCount++;

        // 10. Compute the median of the collected readings.
        //     Copy the valid portion into a scratch array and insertion-sort it.
        float scratch[HR_MEDIAN_SIZE];
        for (uint8_t i = 0; i < _medianCount; i++) scratch[i] = _medianBuf[i];
        for (uint8_t i = 1; i < _medianCount; i++) {
            float key = scratch[i];
            int8_t j  = (int8_t)i - 1;
            while (j >= 0 && scratch[j] > key) {
                scratch[j + 1] = scratch[j];
                j--;
            }
            scratch[j + 1] = key;
        }
        const float medianBPM = scratch[_medianCount / 2];

        // 11. Apply Exponential Moving Average to the median for final smoothing.
        if (_bpmEMA < 1.0f) {
            // First valid reading — seed EMA directly to avoid a startup transient.
            _bpmEMA = medianBPM;
        } else {
            _bpmEMA = HR_EMA_ALPHA * medianBPM + (1.0f - HR_EMA_ALPHA) * _bpmEMA;
        }
        Serial.printf("[HeartBeat] raw=%.1f  median=%.1f  EMA=%.1f  SNR=%.2f"
                      "  (bin %.2f, %.3f Hz)\n",
                      bpm, medianBPM, _bpmEMA, snr, refinedBin, freqHz);
    }
}

void HeartBeat::reset()
{
    memset(_circBuf, 0, sizeof(_circBuf));
    memset(_vReal,   0, sizeof(_vReal));
    memset(_vImag,   0, sizeof(_vImag));
    _circHead      = 0;
    _totalSamp     = 0;
    _bpmEMA        = 0.0f;
    _fingerPresent = false;
    _irValue       = 0;
    // Reset IIR filter state so the new finger placement starts cleanly.
    _hp_xPrev      = 0.0f;
    _hp_yPrev      = 0.0f;
    _lp_yPrev      = 0.0f;
    // Reset median filter buffer.
    memset(_medianBuf, 0, sizeof(_medianBuf));
    _medianIdx   = 0;
    _medianCount = 0;
}
