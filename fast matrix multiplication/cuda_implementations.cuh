#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cuda.h>

__global__ void cu_dot_prod(float* a, float* b, float* c, int n);

void call_kernal(float* a, float* b, float* c, int n, dim3 dim_grid, dim3 dim_block);