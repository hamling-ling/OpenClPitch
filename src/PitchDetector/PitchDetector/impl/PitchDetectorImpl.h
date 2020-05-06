#pragma once

#include "../PitchDetectorDefs.h"

class PitchDetectorImpl
{
public:
	PitchDetectorImpl(int samplingRate, int samplingSize) 
	: _samplingRate(samplingRate), _samplingSize(samplingSize)
	{}
	virtual ~PitchDetectorImpl() = 0;
	virtual bool Initialize() = 0;
	virtual bool Detect(const int16_t* x, PitchInfo& pitch) = 0;
protected:
	const int _samplingRate;
	const int _samplingSize;
};
