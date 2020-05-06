//
//  PitchDetectorClImpl.cpp
//  PitchDetector
//
//  Created by Nobuhiro Kuroiwa on 2020/05/06.
//  Copyright (c) 2020年 Nobuhiro Kuroiwa. All rights reserved.
//

#include "PitchDetectorClImpl.h"

PitchDetectorClImpl::PitchDetectorClImpl(int samplingRate, int samplingSize)
:
PitchDetectorImpl(samplingRate, samplingSize)
{
}

PitchDetectorClImpl::~PitchDetectorClImpl()
{
}

bool PitchDetectorClImpl::Initialize()
{
    return false;
}

bool PitchDetectorClImpl::Detect(const float* x, PitchInfo& pitch)
{
    return false;
}
