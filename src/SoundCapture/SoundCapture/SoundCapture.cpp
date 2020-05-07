#include "SoundCapture.h"

#include <algorithm>
#include <cstring>
#include <time.h>

//#define OUTPUT_TO_FILE

#ifdef OUTPUT_TO_FILE
#include <iostream>// debug
#include <fstream>
#endif

#include "CaptureDevice.h"

using namespace std;

static int NanoSleep(long nano)
{
	struct timespec tim, tim2;
	tim.tv_sec  = 0;
	tim.tv_nsec = nano;
	int result = nanosleep(&tim , &tim2);
	if(result < 0)
	{
		printf("Nano sleep system call failed \n");
		return -1;
	}
   return result;
}

SoundCapture::SoundCapture(int sampleRate, int sampleNum)
: _device(make_shared<CaptureDevice>(sampleRate, sampleNum)),
  _eventHandler(nullptr),
  _leaseFunc(nullptr),
  _finishLeaseFunc(nullptr)
{
}

SoundCapture::~SoundCapture()
{
	Stop();
	if(_device) {
		_device->DestroyDevice();
	}
}

SoundCaptureError SoundCapture::Initialize(
	SoundCaptureBufferRelaseFunc_t 	leaseFunc,
	SoundCaptureBufferFinishFunc_t 	finishLeaseFunc,
	SoundCaptureEventHandler_t     	eventHandler,
	void* 							user)
{
	if(!leaseFunc || !finishLeaseFunc) {
		return SoundCaptureErrorArgs;
	}

	_eventHandler    = eventHandler;
	_leaseFunc       = leaseFunc;
	_finishLeaseFunc = finishLeaseFunc;
	_user = user;

	return SoundCaptureSuccess;
}

SoundCaptureError SoundCapture::Start()
{
	if(_device->SelectedDevice() == -1) {
		return SoundCaptureErrorNoDevice;
	}

	ServiceStart();

	return SoundCaptureSuccess;
}

SoundCaptureError SoundCapture::Stop()
{
	ServiceStop();
	return SoundCaptureSuccess;
}

SoundCaptureError SoundCapture::GetDevices(std::vector<std::string>& vec)
{
	_device->GetDevices(vec);
	return SoundCaptureSuccess;
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

	return SoundCaptureSuccess;
}

SoundCaptureError SoundCapture::DeselectDevice()
{
	if(IsRunning()) {
		return SoundCaptureErrorAlreadyRunning;
	}

	if(_device->SelectedDevice() == -1) {
		return SoundCaptureSuccess;
	}

	CaptureDeviceError err = _device->DestroyDevice();
	if(CaptureDeviceErrorDeviceExist == err) {
		return SoundCaptureErrorInternal;
	} else if(CaptureDeviceErrorNoError != err) {
		return SoundCaptureErrorInternal;
	}

	return SoundCaptureSuccess;
}

int SoundCapture::SelectedDevice()
{
    return _device->SelectedDevice();
}

void SoundCapture::ServiceProc()
{
	const int N = _device->SampleNumber();

	if(_device->SelectedDevice() == -1) {
		MakeCaptureNotification(SoundCaptureErrorNoDevice);
		ProcFinished();
		return;
	}

	_device->CaptureStart();

	while (!IsStopRequested())
	{
		SoundCaptureError result = SoundCaptureSuccess;
		{
			std::lock_guard<std::recursive_mutex> lock(_dataMutex);

			int capturedNum = _device->GetCapturedNum();
			if(capturedNum < N) {
				SleepForSamples(N - capturedNum);
				continue;
			}

			int16_t* data = _leaseFunc(this, _user);
			if(data) {
				CaptureDeviceError deviceError = _device->Sample(data);
				_finishLeaseFunc(this, data, _user);

				if(CaptureDeviceErrorTooEarly == deviceError) {
					continue;
				} else if(CaptureDeviceErrorNoError != deviceError) {
					result = SoundCaptureErrorInternal;
				}
			} else {
				result = SoundCaptureErrorInternal;
			}
		}

		if(!_eventHandler) {
			continue;
		}

		MakeCaptureNotification(result);
	}

	_device->CaptureStop();
    ProcFinished();
}

void SoundCapture::MakeCaptureNotification(SoundCaptureError err)
{
	SoundCaptureNotification note = {};
	note.type = SoundCaptureNotificationTypeCapture;
	note.err  = err;
	if(_eventHandler) {
		_eventHandler(this, note, _user);
	}
}

void SoundCapture::SleepForSamples(const int leftSampleNum)
{
	if(0 < leftSampleNum) {
		float sampleSec     = 1.0f / _device->SamplingRate();
		long  sampleNanoSec = sampleSec * 1000 * 1000 * 1000;
		NanoSleep(leftSampleNum * sampleSec);
	}
}
