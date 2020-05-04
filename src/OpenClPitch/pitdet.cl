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

        //printf("vloat(%d,x) = %f %f %f %f\n", i, a.x, a.y, a.z, a.w);
        float d = dot(a, b);
        /*printf("%d - %d (%f %f %f %f) x (%f %f %f %f)=%f\n",
               global_id, i,
               a.x, a.y, a.z, a.w,
               b.x, b.y, b.z, b.w,
               d);*/
        r += dot(a, b);

        float4 m4 = pow(a, 2.0f) + pow(b, 2.0f);
        float2 m2 = m4.xy + m4.zw;
        m += m2.x + m2.y;
    }

    // copy to ouput buffer for debug
    out[global_id] = 2.0f * r / (m + eps);
    barrier(CLK_LOCAL_MEM_FENCE);
}

__kernel void peak_detection(
    __global float* x,
    __global int2*  max_indices
                   )
{
    const int   global_id  = get_global_id(0);
    const int   group_id   = get_group_id(0);
    const int   local_size = get_local_size(0);
    const int   local_id   = get_local_id(0);

    if(local_id == 0) {

        // stores index of max before and after zero crossing
        int   locmax[2]        = {-1, -1};
        // index to store in status[], 0 before zero crossing, =1 after zero crossing.
        bool  locmax_idx       = 0;
        // theoretical min of m
        float max_val          = -1.0f;

        // inex start if this work group, this value shuld equls to group_id * local_size
        int   global_idx_start = global_id;
        // loop ends before reaches this index
        int   global_idx_end   = global_idx_start + local_size;

        float prev             = x[global_idx_start];
        for(int i = global_idx_start; i < global_idx_end; i++) {
            const float cur = x[i];
            if(max_val < cur) {
                //printf("%d %d %d %d renew max %f -> %f, %f\n",
                //       global_id, group_id, local_id, i, max_val, cur, prev);
                locmax[locmax_idx] = i;
            }

            // if - to + zero crossing ocurred
            bool crossing = (prev < 0.0f && 0.0f <= cur);
            // if this is 1st zero crossing
            if(crossing && locmax_idx == 0) {
                //printf("crossing! %d %d\n", global_id, i);
                // store current max as max value before zero crossing
                locmax[0]  = i;
                // next max will be stored as max value after zero crossing
                locmax_idx = 1;
            }
            prev = cur;
        }

        max_indices[group_id]  = int2(locmax[0], locmax[1]);
    }
}

