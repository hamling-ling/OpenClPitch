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
#include <vector>
#include <memory>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include "../../PeakDetectMachine/PeakDetectMachine.h"

#define CL_PATH         "pitdet.cl"
#define DEVICE_INDEX    0
#define WORK_GROUP_SIZE 128

typedef struct _MachineContext_t MachineContext_t;

class PitchDetectorClImpl : public PitchDetectorImpl
{
public:
    PitchDetectorClImpl(int samplingRate, int samplingSize);
    virtual ~PitchDetectorClImpl();
    virtual bool Initialize();

protected:
    virtual bool Detect(const int16_t* x, PitchInfo& pitch);

private:
    // N samples as an input. Will be replaced when zerocopy implemented
    std::vector<int16_t>     _host_x;
    std::vector<osk_float_t> _host_out;
    const uint16_t           _device_index;
    const float              _sampling_freq;
    const float              _delta_t;
    cl::Device               _device;
    cl::Context              _context;
    cl::CommandQueue         _queue;
    cl::Program              _program;
    cl::NDRange              _range_global;
    cl::NDRange              _range_local;
    cl::Buffer               _device_x;
    cl::Buffer               _device_out;
    std::shared_ptr<cl::make_kernel<cl::Buffer, cl::Buffer>> _kernel;
    MachineContext_t*        _mach;

    bool PeakDetection(const std::vector<osk_float_t>& nsdf, PitchInfo& pitch);
    void PrintException(cl::Error& err);
};

#endif /* defined(__PitchDetector__PitchDetectorClImpl__) */
