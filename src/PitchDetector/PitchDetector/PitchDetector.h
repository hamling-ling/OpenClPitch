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
	bool Detect(const int16_t* x, PitchInfo& pitch);
	int16_t* LeaseBuffer();
	void LeaseFinish(int16_t* buf);

private:
	std::shared_ptr<PitchDetectorImpl> _impl;
	const int _samplingRate;
	const int _samplingSize;
};
