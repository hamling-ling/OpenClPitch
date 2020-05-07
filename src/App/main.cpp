#include <iostream>
#include <vector>
#include "../PitchDetector/PitchDetector/PitchDetector.h"

using namespace std;


int16_t Conv2Int16(float val)
{
    // converting to int16
    return val * 32768;
}

float Conv2Float(int16_t val)
{
    return ((float)val) / 32768.0f;
}

void DebugPrintAutoCorr(const std::vector<int16_t>& x)
{
    cout << "right acorr ----" << endl;
    const int N = x.size();
    for(int tau = 0; tau < N; tau++) {
        float r = 0.0f;
        float m = 0.0f;
        for(int i = 0; i < N-tau; i++) {
            float x_i    = Conv2Int16(x[i]);
            float x_itau = Conv2Int16(x[i+tau]);

            r += x_i * x_itau;
            m += pow(x_i,2.0f) + pow(x_itau,2.0f);
        }
        float nsdf = 2.0f * r / (m + 0.00001f);
        cout << "[" << tau << "] = " << nsdf << endl;
    }
}

void CraeteSineWave(float delta_t, std::vector<int16_t>& x)
{
    // frequency f Hz wave is sin(2*pi*f*t)
    // if f=440Hz, func is sin(2*pi*440*t)
    // for sampling freq 44.1 kHz, 1 samples is 1/(44.1*1000) sec
    int N = x.size();
    for(int i = 0; i < N; i++) {
        float fval = sin(i * delta_t * 440.0 * 2.0 * M_PI);
        // converting to 
        x[i]     =  Conv2Int16(fval);
        //cout << "[" << i << "] = " << h_x[i] << endl;
    }
}


int main(int argc, char *argv[])
{
    const int   SAMPLING_RATE = 44.1*1000;
    const int   N             = 1024;
    const float DELTA_T       = 1.0f/SAMPLING_RATE;

    std::vector<int16_t> x( N);
    CraeteSineWave(DELTA_T, x);

    PitchDetector detector(SAMPLING_RATE, N);
    if(!detector.Initialize()) {
        cout << "PitchDetector initialize failure" << endl;
        return 1;
    }

    PitchInfo pitch = {};
    if(!detector.Detect(&x[0], pitch)) {
        cout << "Pitch Detectin failed" << endl;
        return 1;
    }

    cout << "freq=" << pitch.freq << ", " << pitch.noteStr << endl;
    cout << "freq= " << pitch.freq << "Hz, midi=" << pitch.midi << ", note=" << pitch.noteStr << endl;

    return 0;
}
