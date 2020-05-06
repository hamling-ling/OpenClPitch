//
//  PitchDetectorClImpl.h
//  PitchDetector
//
//  Created by Nobuhiro Kuroiwa on 2020/05/06.
//  Copyright (c) 2020年 Nobuhiro Kuroiwa. All rights reserved.
//

#ifndef __PitchDetector__PitchDetectorClImpl__
#define __PitchDetector__PitchDetectorClImpl__

#include "../impl/PitchDetectorImpl.h"

class PitchDetectorClImpl : public PitchDetectorImpl
{
public:
    PitchDetectorClImpl(int samplingRate, int samplingSize);
    virtual ~PitchDetectorClImpl();
    virtual bool Initialize();
    
protected:
    virtual bool Detect(const float* x, PitchInfo& pitch);
};

#endif /* defined(__PitchDetector__PitchDetectorClImpl__) */
