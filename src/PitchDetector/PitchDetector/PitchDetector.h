#pragma once

#include <memory>
#include "PitchDetectorDefs.h"

class PitchDetectorImpl;

class PitchDetector
{
public:
	PitchDetector(int samplingRate, int samplingSize);
	virtual ~PitchDetector();
	bool Initialize();
	bool Detect(const float* x, PitchInfo& pitch);

private:
	std::shared_ptr<PitchDetectorImpl> _impl;
	const int _samplingRate;
	const int _samplingSize;
};
