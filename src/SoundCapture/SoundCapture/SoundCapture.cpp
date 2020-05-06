#include "SoundCapture.h"

#include <algorithm>
#include <cstring>

//#define USE_VDSP
#ifdef USE_VDSP
#include <Accelerate/Accelerate.h>
#endif

//#define OUTPUT_TO_FILE

#ifdef OUTPUT_TO_FILE
#include <iostream>// debug
#include <fstream>
#endif

#include "CaptureDevice.h"

using namespace std;

SoundCapture::SoundCapture(int sampleRate, int sampleNum)
: _unity(32768.0f), _sampleBuf(NULL), _sampleBufNrm(NULL), _device(make_shared<CaptureDevice>(sampleRate, sampleNum))
{
}

SoundCapture::~SoundCapture()
{
	Stop();
	if(_device) {
		_device->DestroyDevice();
	}
	delete[] (_sampleBuf);
    delete[] (_sampleBufNrm);
}

bool SoundCapture::Initialize(SoundCaptureCallback_t callback, void* user)
{
	if(_sampleBuf) {
		return true;
	}
	
	_callback = callback;
	_sampleBuf = new int16_t[_device->SampleNumber()];
	_user = user;

	if(!_sampleBuf) {
		return false;
	}
	
    _sampleBufNrm = new float[_device->SampleNumber()];
    if(!_sampleBufNrm) {
        return false;
    }
    
	return true;
}

SoundCaptureError SoundCapture::Start()
{
	if(!_sampleBuf) {
		return SoundCaptureErrorNotInitialized;
	}
	
	if(_device->SelectedDevice() == -1) {
		return SoundCaptureErrorNoDevice;
	}
	
	ServiceStart();

	return SoundCaptureErrorNoError;
}

SoundCaptureError SoundCapture::Stop()
{
	ServiceStop();
	return SoundCaptureErrorNoError;
}

SoundCaptureError SoundCapture::GetDevices(std::vector<std::string>& vec)
{
	_device->GetDevices(vec);
	return SoundCaptureErrorNoError;
}

int SoundCapture::Level()
{
	std::lock_guard<std::recursive_mutex> lock(_dataMutex);
    return _level;
}

SoundCaptureError SoundCapture::SelectDevice(int index)
{
	if(IsRunning()) {
		return SoundCaptureErrorAlreadyRunning;
	}
	
	if(_device->SelectedDevice() != -1) {
		return SoundCaptureErrorDeviceExist;
	}
	
	CaptureDeviceError err = _device->CreateDevice(index);
	if(CaptureDeviceErrorDeviceExist == err) {
		return SoundCaptureErrorNoDevice;
	}
	
	return SoundCaptureErrorNoError;
}

SoundCaptureError SoundCapture::DeselectDevice()
{
	if(IsRunning()) {
		return SoundCaptureErrorAlreadyRunning;
	}
	
	if(_device->SelectedDevice() == -1) {
		return SoundCaptureErrorNoError;
	}
	
	CaptureDeviceError err = _device->DestroyDevice();
	if(CaptureDeviceErrorDeviceExist == err) {
		return SoundCaptureErrorInternal;
	}
	
	return SoundCaptureErrorNoError;
}

int SoundCapture::SelectedDevice()
{
    return _device->SelectedDevice();
}

SoundCaptureError SoundCapture::GetBuffer(float* out)
{
	if (out) {
		std::lock_guard<std::recursive_mutex> lock(_dataMutex);
		int N = _device->SampleNumber();
		for (int i = 0; i < N; i++) {
			out[i] = _sampleBufNrm[i];
		}
	}
	return SoundCaptureErrorNoError;
}

float* SoundCapture::GetRawBufferPointer()
{
    return _sampleBufNrm;
}

void SoundCapture::ServiceProc()
{
	const int N = _device->SampleNumber();
	ALshort* data = static_cast<int16_t*>(_sampleBuf);

	if(!_sampleBuf) {
		NotifyCaptureError(SoundCaptureErrorNotInitialized);
		ProcFinished();
		return;
	}
	
	if(_device->SelectedDevice() == -1) {
		NotifyCaptureError(SoundCaptureErrorNoDevice);
		ProcFinished();
		return;
	}
	
	_device->CaptureStart();

	CaptureDeviceError err = CaptureDeviceErrorNoError;
	while (!IsStopRequested())
	{
		{
			std::lock_guard<std::recursive_mutex> lock(_dataMutex);
			
			// Read the captured audio
			memset(data, 0, sizeof(ALshort) * N);
			CaptureDeviceError err = _device->Sample(data);
			if(CaptureDeviceErrorTooEarly == err) {
				continue;
			}
			
			if(CaptureDeviceErrorNoError == err) {
				//  Process/filter captured data
				ProcessData(static_cast<int16_t*>(data), N);
			}
		}

		if(!_callback) {
			continue;
		}
		
		SoundCaptureNotification note;
		if(CaptureDeviceErrorNoError == err) {
			note.type = SoundCaptureNotificationTypeCaptured;
			note.user = _user;
			note.err = SoundCaptureErrorNoError;
			_callback(this, note);
		} else {
			NotifyCaptureError(SoundCaptureErrorInternal);
			break;
		}
	}

	_device->CaptureStop();
    ProcFinished();
}

void SoundCapture::ProcessData(int16_t *data, int dataNum)
{
#ifdef OUTPUT_TO_FILE
	std::ofstream outfile("new.txt", std::ofstream::trunc);
	for (int i = 0; i<dataNum; i++) {
		outfile << int(data[i]) << endl;
	}
#endif
    
    float maxNrmValue = 0;
#ifdef USE_VDSP
    // int16_t -> float
    vDSP_vflt16(data, 1, _sampleBufNrm, 1, dataNum);
    // float -> normalized float
    vDSP_vsdiv(_sampleBufNrm, 1, &_unity, _sampleBufNrm, 1, dataNum);
    // maximum
    vDSP_maxv(_sampleBufNrm, 1, &maxNrmValue, dataNum);
#else
    for (int i = 0; i<dataNum; i++) {
        _sampleBufNrm[i] = data[i] / _unity;
        maxNrmValue = max(_sampleBufNrm[i], maxNrmValue);
    }
#endif
    
	_level = static_cast<int>(maxNrmValue*100.0f);
}

void SoundCapture::NotifyCaptureError(SoundCaptureError err)
{
	SoundCaptureNotification note;
	note.type = SoundCaptureNotificationTypeCaptureError;
	note.err = err;
	if(_callback) {
		_callback(this, note);
	}
}
