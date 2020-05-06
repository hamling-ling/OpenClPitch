#pragma once

#include "../PitchDetectorDefs.h"

class PitchDetectorImpl
{
public:
	PitchDetectorImpl(int samplingRate, int samplingSize);
	virtual ~PitchDetectorImpl() = 0;
	virtual bool Initialize() = 0;
	virtual bool Detect(const float* x, PitchInfo& pitch) = 0;
};
