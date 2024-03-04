#include "fft.h"

#define MAX_SAMPLES SAMPLES

void calc_fft(double input[], int samples) {
    arduinoFFT FFT = arduinoFFT();

    double vImag[MAX_SAMPLES];
    memset(vImag, 0x00, sizeof(vImag));

    /*FFT*/
    FFT.Windowing(input, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(input, vImag, samples, FFT_FORWARD);
    FFT.ComplexToMagnitude(input, vImag, samples);
}
