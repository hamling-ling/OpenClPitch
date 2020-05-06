//
//  RunnableService.cpp
//  SoundCapture
//
//  Created by Nobuhiro Kuroiwa on 2015/04/03.
//  Copyright (c) 2015年 Nobuhiro Kuroiwa. All rights reserved.
//

#include "RunnableService.h"

using namespace std;

RunnableService::RunnableService()
: _isRunning(false), _stopRunning(false)
{
	
}

RunnableService::~RunnableService()
{
	ServiceStop();
}

bool RunnableService::IsRunning()
{
	return _isRunning;
}

RunnableServiceError RunnableService::ServiceStart()
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
 
	if (_isRunning) {
		return RunnableServiceErrorAlreadyRunning;
	}
 
	_isRunning = true;
	_stopRunning = false;
	_thread = thread(&RunnableService::ServiceProc, this);
	
	return RunnableServiceErrorNoError;
}

RunnableServiceError RunnableService::ServiceStop()
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (_isRunning) {
		_stopRunning = true;
	}
	
	if (_thread.joinable()) {
		_thread.join();
	}
	
	return RunnableServiceErrorNoError;
}

/**

 */

bool RunnableService::IsStopRequested()
{
	return _stopRunning;
}

void RunnableService::ProcFinished()
{
	_isRunning = false;
}
