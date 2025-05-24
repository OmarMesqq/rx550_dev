#include <CL/cl.h>
#include <stdio.h>

#define WORK_ITEMS 4

int main() {
    FILE* f = fopen("square.cl", "r");
    if (!f) {
        printf("Failed to read kernel.\n");
        return -1;
    }
    // Read all kernel source code to get its size, then rewind it for later copy
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    char* kernel_src = malloc(size + 1);
    if (!kernel_src) {
        printf("Failed to allocate heap memory for kernel code.\n");
        return -1;
    }
    // Copy contents from kernel source code to memory
    fread(kernel_src, 1, size, f);
    kernel_src[size] = '\0';
    fclose(f);

    // RAM allocated arrays
    float data[WORK_ITEMS] = {1.0, 2.0, 3.0, 4.0};
    float out[WORK_ITEMS];

    cl_platform_id platformId; cl_device_id deviceId; cl_context openClContext;
    cl_program openClProgram; cl_kernel kern;

    // Gets the first OpenCL platform (the GPU)
    clGetPlatformIDs(1, &platformId, NULL);
    // Gets the GPU from this platform
    clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);

    openClContext = clCreateContext(NULL, 1, &deviceId, NULL, NULL, NULL);
    
    // Props of the command queue: where we can submit kernels to run
    cl_queue_properties props[] = {
        CL_QUEUE_PROPERTIES, 
        CL_QUEUE_PROFILING_ENABLE,  // Track GPU time
        0
    };

    cl_command_queue q = clCreateCommandQueueWithProperties(openClContext, deviceId, props, NULL);

    // GPU RAM allocated arrays that receive in and out
    cl_mem din, dout;
    din = clCreateBuffer(openClContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(data), data, NULL);
    dout = clCreateBuffer(openClContext, CL_MEM_WRITE_ONLY, sizeof(out), NULL, NULL);

    // Create an OpenCL program object from kernel source code
    openClProgram = clCreateProgramWithSource(openClContext, 1, &kernel_src, NULL, NULL);

    // Compile the program for the target GPU using the OpenCL runtime and the device driver
    clBuildProgram(openClProgram, 1, &deviceId, NULL, NULL, NULL);

    // Gets the specific kernel from the compiled program (what actually runs on GPU)
    kern = clCreateKernel(openClProgram, "square", NULL);

    // Passes the _global float* in and out parameters
    clSetKernelArg(kern, 0, sizeof(cl_mem), &din);
    clSetKernelArg(kern, 1, sizeof(cl_mem), &dout);

    // How many GPU threads?
    size_t workItems = WORK_ITEMS;
    // OpenCL event for tracking execution time
    cl_event evt;

    // Enqueue the "square" kernel in the command queue "q" to run across "workItems" threads and capture the event "evt"
    clEnqueueNDRangeKernel(q, kern, 1, NULL, &workItems, NULL, 0, NULL, &evt);
    clWaitForEvents(1, &evt);   // Wait for completion

    // Copy contents of GPU RAM "dout" array to RAM "out" array
    clEnqueueReadBuffer(q, dout, CL_TRUE, 0, sizeof(out), out, 0, NULL, NULL);

    // Profiling: get timestamps of kernel execution start and end
    cl_ulong start, end;
    clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    // Print results computed by the GPU and total GPU time
    for (int i = 0; i < WORK_ITEMS; i++) {
        printf("%f^2 = %f\n", data[i], out[i]);
    }
    printf("GPU time (ns): %lu\n", end - start);

    
    free(kernel_src);
}
