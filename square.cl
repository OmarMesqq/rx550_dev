// Declare a kernel: a routine for transforming data in parallel leveraging the GPU
// __global float* in and out are addresses in GPU memory accessible to all threads
__kernel void square(__global float* in, __global float* out) {
    int i = get_global_id(0); // index of the current work item/thread
    out[i] = in[i] * in[i];   // multiply each index by itself to get the square result. Done in parallel.
}
