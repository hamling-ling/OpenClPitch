#include "PitchDetector.h"

#include "impl/PitchDetectorNullImpl.h"
#include "climpl/PitchDetectorClImpl.h"

using namespace std;

PitchDetector::PitchDetector(int samplingRate, int samplingSize)
: _samplingRate(samplingRate), _samplingSize(samplingSize)
{
    std::shared_ptr<PitchDetectorNullImpl> ptr = std::make_shared<PitchDetectorNullImpl>(samplingRate, samplingSize);
    _impl = std::dynamic_pointer_cast<PitchDetectorImpl>(ptr);
}

PitchDetector::~PitchDetector()
{
}

bool PitchDetector::Initialize()
{
    std::shared_ptr<PitchDetectorClImpl> ptr = std::make_shared<PitchDetectorClImpl>(
        _samplingRate, _samplingSize);
    if(ptr->Initialize()) {
        _impl = std::dynamic_pointer_cast<PitchDetectorImpl>(ptr);
        return true;
    }

    return _impl->Initialize();
}

bool PitchDetector::Detect(const int16_t* x, PitchInfo& pitch)
{
    return _impl->Detect(x, pitch);
}

int16_t* PitchDetector::LeaseBuffer()
{
    return _impl->LeaseBuffer();
}

void PitchDetector::LeaseFinish(int16_t* buf)
{
    return _impl->LeaseFinish(buf);
}
