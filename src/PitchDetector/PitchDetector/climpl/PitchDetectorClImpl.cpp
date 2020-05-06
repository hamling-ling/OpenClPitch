//
//  PitchDetectorClImpl.cpp
//  PitchDetector
//
//  Created by Nobuhiro Kuroiwa on 2020/05/06.
//  Copyright (c) 2020年 Nobuhiro Kuroiwa. All rights reserved.
//

#include "PitchDetectorClImpl.h"
#include "util.hpp"
#include "err_code.h"
#include "device_picker.hpp"
#include "../../PeakDetectMachine/PeakDetectMachine.h"

using namespace std;

PitchDetectorClImpl::PitchDetectorClImpl(int samplingRate, int samplingSize)
:
PitchDetectorImpl(samplingRate, samplingSize),
_device_index(DEVICE_ID),
_sampling_freq(samplingRate),
_delta_t(1.0/_sampling_freq)
{
    _host_x   = vector<int16_t>(    samplingSize, 0);
    _host_out = vector<osk_float_t>(samplingSize, 0);
}

PitchDetectorClImpl::~PitchDetectorClImpl()
{
    DestroyPeakDetectMachineContext(_mach);
}

bool PitchDetectorClImpl::Initialize()
{
    _mach = CreatePeakDetectMachineContext();

    std::vector<cl::Device> devices;
    const unsigned numDevices = getDeviceList( devices);

    // Check device index in range
    if ( _device_index >= numDevices)
    {
        std::cout << "Invalid device index\n";
        return false;
    }

    string name;
    getDeviceName( _device, name);
    std::cout << "\nUsing OpenCL device: " << name << "\n";
    _device = devices[_device_index];

    std::vector<cl::Device> chosen_device { _device };

    bool ret = true;
    try
    {
        _context = cl::Context(chosen_device);
        _queue   = cl::CommandQueue( _context, _device);

        // Create the compute program from the source buffer
        _program = cl::Program(_context, util::loadProgram(CL_PATH), true);
        // Create the compute kernel from the program for bit-reverse ordering
        _range_global = cl::NDRange(_samplingSize);
        _range_local  = cl::NDRange(WORK_GROUP_SIZE);
        // order bit reversealy
        _device_x   = cl::Buffer(_context, _host_x.begin(), _host_x.end(), true);
        _device_out = cl::Buffer(_context,
                                 CL_MEM_READ_WRITE,
                                 sizeof(_host_out[0]) * _host_out.size()
                                 );
        _kernel.reset(
            new cl::make_kernel<cl::Buffer, cl::Buffer>(_program, "nsdf")
        );
    }
    catch (cl::Error err) {
        /*std::cout << "Exception\n";
        std::cerr << "ERROR: "
                  << err.what()
                  << "("
                  << err_code(err.err())
                  << ")"
                  << std::endl;*/
        ret = false;
    }
    return ret;
}

bool PitchDetectorClImpl::Detect(const int16_t* x, PitchInfo& pitch)
{
    // will be changed to use zero copy
    cl::copy(_host_x.begin(), _host_x.end(), _device_x);

    auto args = cl::EnqueueArgs( _queue, _range_global, _range_local);
    (*_kernel)(args, _device_x, _device_out);
    _queue.finish();

    cl::copy(_queue, _device_out, _host_out.begin(), _host_out.end());

    return PeakDetection(_host_out, pitch);
}

bool PitchDetectorClImpl::PeakDetection(const std::vector<osk_float_t>& nsdf, PitchInfo& pitch)
{
    ResetMachine(_mach);
    for(auto v : nsdf) {
        Input(_mach, v);
    }

    PeakInfo_t keyMaximums[4] = { 0 };
    int        keyMaxLen      = 0;
    bool       ret            = false;

    GetKeyMaximums(_mach, 0.8f, keyMaximums, sizeof(keyMaximums)/sizeof(keyMaximums[0]), &keyMaxLen);
    if (0 < keyMaxLen) {
        float delta = 0;
        if(ParabolicInterp(_mach, keyMaximums[0].index, &nsdf[0], _samplingSize, &delta)) {
            const float    peri = _delta_t * (keyMaximums[0].index + delta);
            const float    freq = 1.0 / peri;
            const uint16_t midi = (uint16_t)round(log10f(freq / 27.5f) / kNoteConst) + 21;
            cout << "freq= " << freq << " Hz, note=" << kNoteStrings[midi % 12] << endl;

            pitch.freq    = freq;
            pitch.midi    = midi;
            pitch.noteStr = kNoteStrings[midi % 12];
        }
        ret = true;
    }

    pitch.error = ret;
    return ret;
}