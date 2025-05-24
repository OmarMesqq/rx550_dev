#include <CL/cl.h>
#include <stdio.h>

const char *kernel_src =
"__kernel void square(__global float* in, __global float* out) {\n"
"   int i = get_global_id(0);\n"
"   out[i] = in[i] * in[i];\n"
"}\n";

int main() {
    float data[4] = {1.0, 2.0, 3.0, 4.0}, out[4];
    cl_platform_id pid; cl_device_id did; cl_context ctx;
    cl_program prog; cl_kernel kern;
    cl_mem din, dout;

    clGetPlatformIDs(1, &pid, NULL);
    clGetDeviceIDs(pid, CL_DEVICE_TYPE_GPU, 1, &did, NULL);
    ctx = clCreateContext(NULL, 1, &did, NULL, NULL, NULL);
    cl_queue_properties props[] = {
    CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE,
        0
    };
    
    cl_command_queue q = clCreateCommandQueueWithProperties(ctx, did, props, NULL);


    din = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(data), data, NULL);
    dout = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof(out), NULL, NULL);

    prog = clCreateProgramWithSource(ctx, 1, &kernel_src, NULL, NULL);
    clBuildProgram(prog, 1, &did, NULL, NULL, NULL);
    kern = clCreateKernel(prog, "square", NULL);

    clSetKernelArg(kern, 0, sizeof(cl_mem), &din);
    clSetKernelArg(kern, 1, sizeof(cl_mem), &dout);

    size_t global = 4;
    cl_event evt;
    clEnqueueNDRangeKernel(q, kern, 1, NULL, &global, NULL, 0, NULL, &evt);
    clWaitForEvents(1, &evt);
    clEnqueueReadBuffer(q, dout, CL_TRUE, 0, sizeof(out), out, 0, NULL, NULL);

    cl_ulong start, end;
    clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    for (int i = 0; i < 4; i++) {
        printf("%f^2 = %f\n", data[i], out[i]);
    }

    printf("GPU time (ns): %lu\n", end - start);
}
