//
//  RunnableService.h
//  SoundCapture
//
//  Created by Nobuhiro Kuroiwa on 2015/04/03.
//  Copyright (c) 2015年 Nobuhiro Kuroiwa. All rights reserved.
//

#ifndef __SoundCapture__RunnableService__
#define __SoundCapture__RunnableService__

#include <thread>
#include <mutex>

typedef enum _RunnableServiceError {
	RunnableServiceErrorNoError,
	RunnableServiceErrorAlreadyRunning,
	RunnableServiceErrorInternal,
} RunnableServiceError;

class RunnableService
{
public:
	RunnableService();
	virtual ~RunnableService();
	bool IsRunning();
	
protected:
	virtual RunnableServiceError ServiceStart();
	virtual RunnableServiceError ServiceStop();
	virtual void ServiceProc() = 0;
	bool IsStopRequested();
	void ProcFinished();
private:
	std::thread _thread;
	bool _isRunning;
	bool _stopRunning;
	std::recursive_mutex _mutex;
};

#endif /* defined(__SoundCapture__RunnableService__) */
