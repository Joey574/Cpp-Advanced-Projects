#include "cuda_implementations.cuh"

__global__ void cu_dot_prod(float* a, float* b, float* c, int n) {
	int i = blockDim.x * blockIdx.x + threadIdx.x;

	int row = i / n;
	int col = i % n;

	if (row < n) {
		float sum = 0.0f;

		for (int k = 0; k < n; k++) {
			sum = __fmaf_ieee_rn(a[row * n + k], b[k * n + col], sum);
			//sum += a[i * n + k] * b[k * n + j];
		}

		c[row * n + col] = sum;
	}
}

void call_kernal(float* a, float* b, float* c, int n, dim3 dim_grid, dim3 dim_block) {
	cu_dot_prod <<< dim_grid, dim_block >>> (a, b, c, n);
}