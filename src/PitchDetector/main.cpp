//
//  main.cpp
//  OpenClPitch
//
//  Created by Nobuhiro Kuroiwa on 2020/05/01.
//  Copyright © 2020 Nobuhiro Kuroiwa. All rights reserved.
//

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__)
// place cl.hpp to local directory for Mac OS
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#include "util.hpp"
#include "err_code.h"
#include "device_picker.hpp"
#include "PeakDetectMachine.h"
#include "StopWatch.h"

//------------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------

#if defined(__APPLE__)
// my personal debug setting. need to change later
#define CL_PATH         "/Users/nobu/GitHub/OpenClPitch/src/OpenClPitch/pitdet.cl"
#define DEVICE_ID       1 // my personal debugging value
#else
#define CL_PATH         "pitdet.cl"
#define DEVICE_ID       0
#endif
#define SAMPLE_SIZE_N   1024                    // Sample Size
#define WORK_GROUP_SIZE 128                     // Workgroup size
#define SAMPLING_FREQ   (44.1f*1000.0f)         // Sampling Rate in Hz(CD=44.1kHz)
#define DELTA_T         (1.0f/SAMPLING_FREQ)    // Time nterval for x[n]-x[n+1] in sec

//------------------------------------------------------------------------------
//  functions
//------------------------------------------------------------------------------
extern double wtime();   // Returns time since some fixed past point (wtime.c)

using namespace std;

int16_t conv2int16(float val)
{
    // converting to int16
    return val * 32768;
}

float conv2float(int16_t val)
{
    return ((float)val) / 32768.0f;
}

int main(int argc, char *argv[])
{
    const int          N = SAMPLE_SIZE_N;  // Real data sampling size
    std::vector<int16_t> h_x( N);          // N samples as an input

    // frequency f Hz wave is sin(2*pi*f*t)
    // if f=440Hz, func is sin(2*pi*440*t)
    // for sampling freq 44.1 kHz, 1 samples is 1/(44.1*1000) sec
    for(int i = 0; i < N; i++) {
        float fval = sin(i * DELTA_T * 440.0 * 2.0 * M_PI);
        // converting to 
        h_x[i]     =  conv2int16(fval);
        //cout << "[" << i << "] = " << h_x[i] << endl;
    }

    cout << "right acorr ----" << endl;
    for(int tau = 0; tau < N; tau++) {
        float r = 0.0f;
        float m = 0.0f;
        for(int i = 0; i < N-tau; i++) {
            float h_x_i    = conv2float(h_x[i]);
            float h_x_itau = conv2float(h_x[i+tau]);

            r += h_x_i * h_x_itau;
            //cout << "[" << i << "] x [" << i+tau << "] = " << h_x_i * h_x_itau << endl;
            m += pow(h_x_i,2.0f) + pow(h_x_itau,2.0f);
        }
        float nsdf = 2.0f * r / (m + 0.00001f);
        //cout << "[" << tau << "] = " << nsdf << endl;
    }

    try
    {
        cl_uint deviceIndex = DEVICE_ID;

        // Get list of devices
        std::vector<cl::Device> devices;
        unsigned numDevices = getDeviceList( devices);

        // Check device index in range
        if ( deviceIndex >= numDevices)
        {
            std::cout << "Invalid device index\n";
            return EXIT_FAILURE;
        }

        cl::Device device = devices[deviceIndex];

        std::string name;
        getDeviceName( device, name);
        std::cout << "\nUsing OpenCL device: " << name << "\n";

        std::vector<cl::Device> chosen_device;
        chosen_device.push_back( device);

        cl::Context      context( chosen_device);
        cl::CommandQueue queue( context, device);

        // Create the compute program from the source buffer
        cl::Program program = cl::Program(context, util::loadProgram(CL_PATH), true);
        // Create the compute kernel from the program for bit-reverse ordering
        cl::NDRange global(N);
        cl::NDRange local(WORK_GROUP_SIZE);
        // order bit reversealy

        std::vector<osk_float_t> h_out(N, 0);
        cl::Buffer d_x   = cl::Buffer(context, h_x.begin(), h_x.end(), true);
        cl::Buffer d_out = cl::Buffer(context,
                                      CL_MEM_READ_WRITE,
                                      sizeof(h_out[0]) * h_out.size()
                                      );
        cl::make_kernel<cl::Buffer, cl::Buffer>
            nsdf( program, "nsdf");

        // compute nsdf
        nsdf(cl::EnqueueArgs( queue, global, local), d_x, d_out);
        queue.finish();
        cl::copy(queue, d_out, h_out.begin(), h_out.end());

        cout << "nsdf ----" << endl;
        for(int i = 0; i < h_out.size(); i++) {
            cout << "[" << i << "] = " << h_out[i] << endl;
        }

        MachineContext_t* mach = CreatePeakDetectMachineContext();
        StopWatch<std::chrono::milliseconds> sw;
        for(auto v : h_out) {
            Input(mach, v);
        }

        PeakInfo_t keyMaximums[4] = { 0 };
        int keyMaxLen = 0;
        GetKeyMaximums(mach, 0.8f, keyMaximums, sizeof(keyMaximums)/sizeof(keyMaximums[0]), &keyMaxLen);
        if (0 < keyMaxLen) {
            float delta = 0;
            if(ParabolicInterp(mach, keyMaximums[0].index, &h_out[0], N, &delta)) {
                const float    peri = DELTA_T * (keyMaximums[0].index + delta);
                const float    freq = 1.0 / peri;
                const float    k    = log10f(pow(2.0f, 1.0f / 12.0f));
                const uint16_t midi = (uint16_t)round(log10f(freq / 27.5f) / k) + 21;
                cout << "freq= " << freq << " Hz, note=" << kNoteStrings[midi % 12] << endl;
            }
        }
        DestroyPeakDetectMachineContext(mach);
    } catch (cl::Error err) {
        std::cout << "Exception\n";
        std::cerr << "ERROR: "
                  << err.what()
                  << "("
                  << err_code(err.err())
                  << ")"
                  << std::endl;
    }

    return EXIT_SUCCESS;
}
