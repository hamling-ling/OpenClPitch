#pragma OPENCL EXTENSION cl_khr_fp16 : enable

/**
 * FFT Main function
 */
__kernel void nsdf(
    __global half* x,
    __global half* out
                   )
{
    const float eps        = 0.00001f;
    const int   global_id  = get_global_id(0);
    const int   N          = get_global_size(0);

    const int tau = global_id;
    float      r  = 0.0f;
    float      m  = 0.0f;

    for(int i = 0; i < N - tau; i+=4) {
        half4 a = vload4(0, x+i);
        half4 b = vload4(0, x+i+tau);
        r += dot(a, b);

        half4 m4 = pow(a, 2.0f) + pow(b, 2.0f);
        half2 m2 = m4.xy + m4.zw;
        m += m2.x + m2.y;
    }

    // copy to ouput buffer for debug
    out[global_id] = 2.0f * r / (m + eps);
}
