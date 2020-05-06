#pragma OPENCL EXTENSION cl_khr_fp16 : enable

/**
 * FFT Main function
 */
__kernel void nsdf(
    __global short* x,
    __global half*  out
                   )
{
    const float eps        = 0.00001f;
    const int   global_id  = get_global_id(0);
    const int   N          = get_global_size(0);

    const int tau = global_id;
    float      r  = 0.0f;
    float      m  = 0.0f;

    const half8 scale = (1.0f/32768.0f);
    for(int i = 0; i < N - tau; i+=4) {
        short8 sa = vload8(0, x+i);
        short8 sb = vload8(0, x+i+tau);

        half8 a = convert_half8(sa) * scale;;
        half8 b = convert_half8(sb) * scale;
        r += dot(a.s0123, b.s0123) + dot(a.s4567, b.s4567);

        half8 mm8 = pow(a, 2.0f) + pow(b, 2.0f);
        half4 mm4 = mm8.s0123 + mm8.s4567;
        half2 mm2 = mm4.s01 + mm4.s23;
        m += mm2.x + mm2.y;
    }

    // copy to ouput buffer for debug
    out[global_id] = 2.0f * r / (m + eps);
}
