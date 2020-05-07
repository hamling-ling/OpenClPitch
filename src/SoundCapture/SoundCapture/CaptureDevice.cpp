//
//  CaptureDevice.cpp
//  SoundCapture
//
//  Created by Nobuhiro Kuroiwa on 2015/04/03.
//  Copyright (c) 2015年 Nobuhiro Kuroiwa. All rights reserved.
//

#include "CaptureDevice.h"
#include <cstring>

using namespace std;

CaptureDevice::CaptureDevice(int sampleRate, int sampleNum)
: _sampleRate(sampleRate), _sampleNum(sampleNum), _selectedDeviceIndex(-1),
_dev(NULL), _ctx(NULL)
{
}

CaptureDevice::~CaptureDevice()
{
	DestroyDevice();
}

int CaptureDevice::SelectedDevice()
{
	if(_dev) {
		return CaptureDeviceErrorDeviceExist;
	}
	
	return _selectedDeviceIndex;
}

CaptureDeviceError CaptureDevice::GetDevices(std::vector<std::string>& vec)
{
    const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (pDeviceList)
    {
        while (*pDeviceList)
        {
            string name(pDeviceList);
            vec.push_back(name);
            pDeviceList += strlen(pDeviceList) + 1;
        }
    }
    
	return CaptureDeviceErrorNoError;
}

CaptureDeviceError CaptureDevice::CreateDevice(int index)
{
	if(_dev) {
		return CaptureDeviceErrorDeviceExist;
	}
	
	/* If you don't need 3D spatialization, this should help processing time */
	alDistanceModel(AL_NONE);
	_dev = alcCaptureOpenDevice(NULL, _sampleRate, AL_FORMAT_MONO16, sizeof(ALshort)*_sampleNum);
	
	if(_dev) {
		_ctx = alcCreateContext(_dev, NULL);
		alcMakeContextCurrent(_ctx);
		_selectedDeviceIndex = index;
	}
	
	return CaptureDeviceErrorNoError;
}

CaptureDeviceError CaptureDevice::DestroyDevice()
{
	if(_dev) {
		return CaptureDeviceErrorNoError;
	}
	
	CaptureStop();
	
	/* Shutdown and cleanup */
	alcCaptureCloseDevice(_dev);
	
	alcMakeContextCurrent(NULL);
	alcDestroyContext(_ctx);
	
	_dev = NULL;
	_ctx = NULL;
	_selectedDeviceIndex = -1;
	
	return CaptureDeviceErrorNoError;
}

CaptureDeviceError CaptureDevice::CaptureStart()
{
	if(!_dev) {
		return CaptureDeviceErrorNoDevice;
	}
	
	alcMakeContextCurrent(_ctx);
	
	/* Start capture, and enter the audio loop */
	alcCaptureStart(_dev);    //starts ring buffer
	
	return CaptureDeviceErrorNoError;
}

CaptureDeviceError CaptureDevice::CaptureStop()
{
	if(!_dev) {
		return CaptureDeviceErrorNoError;
	}
	
	alcMakeContextCurrent(_ctx);
	alcCaptureStop(_dev);
	
	return CaptureDeviceErrorNoError;
}

int CaptureDevice::SamplingRate()
{
	return _sampleRate;
}

int CaptureDevice::SampleNumber()
{
	return _sampleNum;
}

int CaptureDevice::GetCapturedNum()
{
	// Check how much audio data has been captured (note that
	// 'capturedFrameNum' is the number of frames, not bytes)
	ALint capturedFrameNum;
	alcGetIntegerv(_dev, ALC_CAPTURE_SAMPLES, 1, &capturedFrameNum);

	return capturedFrameNum;
}

CaptureDeviceError CaptureDevice::Sample(ALshort* data)
{
	if(!_dev) {
		return CaptureDeviceErrorNoDevice;
	}
	
	alcMakeContextCurrent(_ctx);

	int capturedSize = GetCapturedNum();
	if(capturedSize < _sampleNum) {
		return CaptureDeviceErrorTooEarly;
	}
	
	alcCaptureSamples(_dev, data, _sampleNum);
	
	return CaptureDeviceErrorNoError;
}
