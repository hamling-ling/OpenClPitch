#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <thread>
#include <mutex>

#include "RunnableService.h"

typedef enum _SoundCaptureError {
	SoundCaptureSuccess,
	SoundCaptureErrorArgs,
	SoundCaptureErrorAlreadyRunning,
	SoundCaptureErrorNotInitialized,
	SoundCaptureErrorNoDevice,
	SoundCaptureErrorDeviceExist,
	SoundCaptureErrorInternal,
} SoundCaptureError;

typedef enum _SoundCaptureNotificationType {
	SoundCaptureNotificationTypeCapture
} SoundCaptureNotificationType;

struct SoundCaptureNotification {
	SoundCaptureNotificationType type;
	SoundCaptureError            err;
};

class SoundCapture;
typedef std::function <void(SoundCapture*, SoundCaptureNotification, void*)> SoundCaptureEventHandler_t;
typedef std::function <int16_t*(SoundCapture*, void*)> SoundCaptureBufferRelaseFunc_t;
typedef std::function <void(SoundCapture*, int16_t*, void*)>     SoundCaptureBufferFinishFunc_t;

class CaptureDevice;

class SoundCapture : public RunnableService
{
public:
	SoundCapture(int sampleRate, int sampleNum);
	virtual ~SoundCapture();

	SoundCaptureError Initialize(SoundCaptureBufferRelaseFunc_t leaseFunc,
								 SoundCaptureBufferFinishFunc_t finishLease,
								 SoundCaptureEventHandler_t     eventHandler,
								 void* 							user);
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
    
protected:
		void ServiceProc();
	
private:
	std::recursive_mutex           _dataMutex;
	int                            _level;
	void*                          _user;
	SoundCaptureEventHandler_t     _eventHandler;
	SoundCaptureBufferRelaseFunc_t _leaseFunc;
	SoundCaptureBufferFinishFunc_t _finishLeaseFunc;
	std::shared_ptr<CaptureDevice> _device;  // never be NULL
	
	void MakeCaptureNotification(SoundCaptureError err);
	void SleepForSamples(const int leftSampleNum);
};

