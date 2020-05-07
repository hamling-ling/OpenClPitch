#include <iostream>
#include <vector>
#include <thread>
#include "../SoundCapture/SoundCapture/SoundCapture.h"
#include "../PitchDetector/PitchDetector/PitchDetector.h"

using namespace std;

const int   SAMPLING_RATE = 44.1*1000;
const int   N             = 1024;
const float DELTA_T       = 1.0f/SAMPLING_RATE;

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

typedef struct _CaptureContext {
    uint32_t       count;
    PitchDetector* detector;
} CaptureContext_t;

int16_t* LeaseBuffer(SoundCapture* cap, void* context)
{
    CaptureContext_t* capCtx = static_cast<CaptureContext_t*>(context);
    return nullptr;
}

void FinishLease(SoundCapture* cap, void* context)
{
    CaptureContext_t* capCtx = static_cast<CaptureContext_t*>(context);
}

void OnCaptured(SoundCapture* cap, SoundCaptureNotification note, void* context)
{
    CaptureContext_t* capCtx = static_cast<CaptureContext_t*>(context);

    PitchInfo pitch = {};
    if(!capCtx->detector->Detect(NULL, pitch)) {
        //cout << "Pitch Detection failed" << endl;
        return;
    }

    cout << "freq= " << pitch.freq << "Hz, midi=" << pitch.midi << ", note=" << pitch.noteStr << endl;
}

int DebugDetector()
{
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

    cout << "freq= " << pitch.freq << "Hz, midi=" << pitch.midi << ", note=" << pitch.noteStr << endl;

    return 0;
}

int main(int argc, char *argv[])
{
    PitchDetector detector(SAMPLING_RATE, N);
    if(!detector.Initialize()) {
        cout << "PitchDetector initialize failure" << endl;
        return 1;
    }

    SoundCapture cap(SAMPLING_RATE, N);

    CaptureContext_t ctx = {};
    ctx.count    = 0;
    ctx.detector = &detector;

    SoundCaptureError   capResult;
    capResult = cap.Initialize(LeaseBuffer, FinishLease, OnCaptured, &ctx);
    if(SoundCaptureSuccess != capResult) {
        cout << "SoundCapture initialize failed " << capResult << endl;
        return 1;
    }

    vector<std::string> devices;
    capResult = cap.GetDevices(devices);
    if(SoundCaptureSuccess != capResult) {
        cout << "Get OpenAL device failed " << capResult << endl;
        return 1;
    }
    if(devices.size() == 0) {
        cout << "OpenAL device not found" << endl;
        return 1;
    }

    for(int i = 0; i < devices.size(); i++) {
        cout << "[" << i << "] " << devices[i] << endl;
    }
    cout << "using device [0] " << devices[0] << endl;

    cap.SelectDevice(0);
    cap.Start();

    cout << "enter to exit" << endl;

    string in;
    cin >> in;
    cout << "exit" << endl;

    cap.Stop();
    this_thread::sleep_for(std::chrono::milliseconds(200));
    capResult = cap.DeselectDevice();
    if(SoundCaptureSuccess != capResult) {
        cout << "OpenAL Device deselect failed " << capResult << endl;
        return 1;
    }

    return 0;
}
