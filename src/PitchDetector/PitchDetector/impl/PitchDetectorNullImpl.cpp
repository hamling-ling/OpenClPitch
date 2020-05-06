//
//  PitchDetectorNullImpl.cpp
//  PitchDetector
//
//  Created by Nobuhiro Kuroiwa on 2015/04/09.
//  Copyright (c) 2015年 Nobuhiro Kuroiwa. All rights reserved.
//

#include "PitchDetectorNullImpl.h"

PitchDetectorNullImpl::PitchDetectorNullImpl(int samplingRate, int samplingSize)
:
PitchDetectorImpl(samplingRate, samplingSize)
{
}

PitchDetectorNullImpl::~PitchDetectorNullImpl()
{
}

bool PitchDetectorNullImpl::Initialize()
{
    return false;
}

bool PitchDetectorNullImpl::Detect(const float* x, PitchInfo& pitch)
{
    return false;
}
