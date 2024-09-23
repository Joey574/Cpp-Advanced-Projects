#include "cuda_implementations.cuh"

__global__ void cu_dot_prod(float* a, float* b, float* c, int n) {
	int i = blockDim.y * blockIdx.y + threadIdx.y;
	int j = blockDim.x * blockIdx.x + threadIdx.x;

	if (i < n && j < n) {
		float sum = 0.0f;

		for (int k = 0; k < n; k++) {
			sum += a[i * n + k] * b[k * n + j];
		}

		c[i * n + j] = sum;
	}
}

void call_kernal(float* a, float* b, float* c, int n, dim3 dim_grid, dim3 dim_block) {
	cu_dot_prod <<< dim_grid, dim_block >>> (a, b, c, n);
}