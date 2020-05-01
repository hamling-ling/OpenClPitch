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

//------------------------------------------------------------------------------
//  Constants
//------------------------------------------------------------------------------
#if defined(__APPLE__)
// my personal debug setting. need to change later
#define CL_PATH         "/Users/nobu/GitHub/OpenClPitch/src/OpenClPitch/pitdet.cl"
#else
#define CL_PATH         "pitdet.cl"
#endif
#define SAMPLE_SIZE_N   8       // Sample Size
#define LOG2N           3       // log2(SAMPLE_SIZE_N)

//------------------------------------------------------------------------------
//  functions
//------------------------------------------------------------------------------
extern double wtime();   // Returns time since some fixed past point (wtime.c)

using namespace std;

int main(int argc, char *argv[])
{
    const int          N = SAMPLE_SIZE_N;  // Real data sampling size
    std::vector<float> h_x( N);            // N samples as an input

    try
    {
        cl_uint deviceIndex = 0;

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
        // order bit reversealy
        cl::Buffer  d_x = cl::Buffer(context,
                                     CL_MEM_READ_WRITE,
                                     sizeof(h_x[0]) * h_x.size()
                                     );
        cl::make_kernel<cl::Buffer> pitdet( program, "nsdf");
        // compute bit-reverse ordering
        pitdet(cl::EnqueueArgs( queue, global), d_x);
        queue.finish();

        cl::copy(queue, d_x, h_x.begin(), h_x.end());
        cout << "output --" << endl;
        for( int i = 0; i < h_x.size(); i++) {
            cout << "[" << i/2 << "] = " << h_x[i] << endl;
        }
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
