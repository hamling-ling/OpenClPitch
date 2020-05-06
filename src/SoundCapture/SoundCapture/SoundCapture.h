#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <thread>
#include <mutex>

#include "RunnableService.h"

typedef enum _SoundCaptureError {
	SoundCaptureErrorNoError,
	SoundCaptureErrorAlreadyRunning,
	SoundCaptureErrorNotInitialized,
	SoundCaptureErrorNoDevice,
	SoundCaptureErrorDeviceExist,
	SoundCaptureErrorInternal,
} SoundCaptureError;

typedef enum _SoundCaptureNotificationType {
	SoundCaptureNotificationTypeCaptured,
	SoundCaptureNotificationTypeCaptureError,
} SoundCaptureNotificationType;

struct SoundCaptureNotification {
	SoundCaptureNotificationType type;
	SoundCaptureError err;
	void* user;
};

class SoundCapture;
typedef std::function < void(SoundCapture*, SoundCaptureNotification)> SoundCaptureCallback_t;

class CaptureDevice;

class SoundCapture : public RunnableService
{
public:
	SoundCapture(int sampleRate, int sampleNum);
	virtual ~SoundCapture();

	bool Initialize(SoundCaptureCallback_t callback, void* user);
	SoundCaptureError Start();
	SoundCaptureError Stop();
	SoundCaptureError GetDevices(std::vector<std::string>& vec);
	SoundCaptureError SelectDevice(int index);
	SoundCaptureError DeselectDevice();
    int SelectedDevice();
    
	/**
	 *	Get recording signal level.
	 */
	int Level();

	/**
	 *	safely obtain captured data. data is normalized.
	 */
	SoundCaptureError GetBuffer(float* out);
    
    float* GetRawBufferPointer();
    
protected:
		void ServiceProc();
	
private:
    const float _unity;
	int16_t* _sampleBuf;
    float* _sampleBufNrm;
	std::recursive_mutex _dataMutex;
	int _level;
	void* _user;
	SoundCaptureCallback_t _callback;
	std::shared_ptr<CaptureDevice> _device;  // never be NULL
	
	void ProcessData(int16_t* data, int dataNum);
	void NotifyCaptureError(SoundCaptureError err);
};

