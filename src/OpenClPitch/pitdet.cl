/**
 * FFT Main function
 */
__kernel void nsdf(
    __global float* x,
    __global float* out
                   )
{
    const float eps        = 0.00001f;
    const int   global_id  = get_global_id(0);
    const int   N          = get_global_size(0);

    const int tau = global_id;
    float      r  = 0.0f;
    float      m  = 0.0f;

    for(int i = 0; i < N - tau; i+=4) {
        float4 a = vload4(0, x+i);
        float4 b = vload4(0, x+i+tau);
        r += dot(a, b);

        float4 m4 = pow(a, 2.0f) + pow(b, 2.0f);
        float2 m2 = m4.xy + m4.zw;
        m += m2.x + m2.y;
    }

    // copy to ouput buffer for debug
    out[global_id] = 2.0f * r / (m + eps);
}
