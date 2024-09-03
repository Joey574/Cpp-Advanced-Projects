#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <immintrin.h>
#include <omp.h>

#define BLOCK_SIZE 64

struct matrix {
	size_t rows;
	size_t columns;
	std::vector<float> matrix;
};

std::string matrix_to_string(matrix a);
void random_init(matrix& a);

void run_test(std::string name, matrix(*dot)(const matrix&, const matrix&));

matrix bad_dot_prod(const matrix& a, const matrix& b);
matrix base_dot_prod(const matrix& a, const matrix& b);
matrix parallel_dot_prod(const matrix& a, const matrix& b);
matrix simd_dot_prod(const matrix& a, const matrix& b);
matrix simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);
matrix simd_ma_unrolled_parallel_dot_prod(const matrix& a, const matrix& b);
matrix blocked_dot_prod(const matrix& a, const matrix& b);
matrix blocked_simd_dot_prod(const matrix& a, const matrix& b);
matrix blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);
matrix divide_conquer_dot_prod(const matrix& a, const matrix& b);

int main()
{
	matrix a; a.rows = 16; a.columns = 16;
	matrix b; b.rows = 16; b.columns = 16;

	a.matrix = std::vector<float>(a.rows * a.columns);
	b.matrix = std::vector<float>(b.rows * b.columns);

	for (int r = 0; r < a.rows; r++) {
		for (int c = 0; c < a.columns; c++) {
			a.matrix[r * a.columns + c] = r;
			b.matrix[r * a.columns + c] = r;
		}
	}

	std::cout << "div:\n" << matrix_to_string(divide_conquer_dot_prod(a, b));
	return 0;

	//run_test("bad_dot_prod", bad_dot_prod);
	run_test("base_dot_prod", base_dot_prod);
	run_test("parallel_dot_prod", parallel_dot_prod);
	run_test("simd_dot_prod", simd_dot_prod);
	run_test("simd_ma_unrolled_dot_prod", simd_ma_unrolled_dot_prod);
	run_test("simd_ma_unrolled_parallel_dot_prod", simd_ma_unrolled_parallel_dot_prod);
	run_test("blocked_dot_prod", blocked_dot_prod);
	run_test("blocked_simd_dot_prod", blocked_simd_dot_prod);
	run_test("blocked_simd_ma_unrolled_dot_prod", blocked_simd_ma_unrolled_dot_prod);
}

std::string matrix_to_string(matrix a) {
	std::string out = "";

	for (int r = 0; r < a.rows; r++) {
		for (int c = 0; c < a.columns; c++) {
			out.append(std::to_string(a.matrix[r * a.columns + c])).append(" ");
		}
		out.append("\n");
	}

	return out;
}
void random_init(matrix& a) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

	a.matrix = std::vector<float>(a.rows * a.columns);
	for (int i = 0; i < a.rows * a.columns; i++) {
		a.matrix[i] = dist(gen);
	}
}

void run_test(std::string name, matrix(*dot)(const matrix&, const matrix&)) {

	std::chrono::duration<double, std::milli> time;

	const int max_size = 1024;
	const int multiplier = 2;
	const int runs = 8;

	matrix c;

	std::cout << name << ":\n";
	for (int size = 8; size <= max_size; size *= multiplier) {

		matrix a; a.rows = size; a.columns = size;
		matrix b; b.rows = size; b.columns = size;

		random_init(a);
		random_init(b);

		double best[runs];

		for (int i = 0; i < runs; i++) {
			auto start = std::chrono::high_resolution_clock::now();
			c = (*dot)(a, b);
			time = std::chrono::high_resolution_clock::now() - start;
			best[i] = (std::chrono::high_resolution_clock::now() - start).count();
		}

		double min = *std::min_element(&best[0], &best[runs]);
		double max = *std::max_element(&best[0], &best[runs]);

		std::cout << "\t" << size << "x" << size << ": " << (min / 1000000.00) << "ms - " << (max / 1000000.00) << "ms\n";
	}
}

void add_matrices(const matrix& a, const matrix& b, matrix& c) {
	int i = 0;
	for (; i + 8 < a.matrix.size(); i += 8) {
		_mm256_store_ps(&c.matrix[i], 
			_mm256_add_ps(
				_mm256_load_ps(&a.matrix[i]),
				_mm256_load_ps(&b.matrix[i])
		));
	}

	for (; i < a.matrix.size(); i++) {
		c.matrix[i] = a.matrix[i] + b.matrix[i];
	}
}

matrix bad_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.columns; j++) {
			for (size_t k = 0; k < b.rows; k++) {
				c.matrix[i * b.columns + j] += a.matrix[i * a.columns + j] * b.matrix[k * b.columns + j];
			}
		}
	}

	return c;
}
matrix base_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			for (size_t k = 0; k < b.columns; k++) {
				c.matrix[i * b.columns + k] += a.matrix[i * a.columns + k] * b.matrix[j * b.columns + k];
			}
		}
	}

	return c;
}
matrix parallel_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			for (size_t k = 0; k < b.columns; k++) {
				c.matrix[i * b.columns + k] += a.matrix[i * a.columns + k] * b.matrix[j * b.columns + k];
			}
		}
	}

	return c;
}
matrix simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			size_t k = 0;
			for (; k + 8 < b.columns; k += 8) {
				_mm256_store_ps(&c.matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						_mm256_load_ps(&a.matrix[i * a.columns + k]),
						_mm256_load_ps(&b.matrix[j * b.columns + k]),
						_mm256_load_ps(&c.matrix[i * b.columns + k])
						));
			}

			for (; k < b.columns; k++) {
				c.matrix[i * b.columns + k] += a.matrix[i * a.columns + k] * b.matrix[j * b.columns + k];
			}
		}
	}

	return c;
}
matrix simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			size_t k = 0;
			for (; k + 16 < b.columns; k += 8) {
				_mm256_store_ps(&c.matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						_mm256_load_ps(&a.matrix[i * a.columns + k]),
						_mm256_load_ps(&b.matrix[j * b.columns + k]),
						_mm256_load_ps(&c.matrix[i * b.columns + k])
					));

				k += 8;
				_mm256_store_ps(&c.matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						_mm256_load_ps(&a.matrix[i * a.columns + k]),
						_mm256_load_ps(&b.matrix[j * b.columns + k]),
						_mm256_load_ps(&c.matrix[i * b.columns + k])
					));
			}

			for (; k < b.columns; k++) {
				c.matrix[i * b.columns + k] += a.matrix[i * a.columns + k] * b.matrix[j * b.columns + k];
			}
		}
	}

	return c;
}
matrix simd_ma_unrolled_parallel_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			size_t k = 0;
			for (; k + 16 < b.columns; k += 8) {
				_mm256_store_ps(&c.matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						_mm256_load_ps(&a.matrix[i * a.columns + k]),
						_mm256_load_ps(&b.matrix[j * b.columns + k]),
						_mm256_load_ps(&c.matrix[i * b.columns + k])
					));

				k += 8;
				_mm256_store_ps(&c.matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						_mm256_load_ps(&a.matrix[i * a.columns + k]),
						_mm256_load_ps(&b.matrix[j * b.columns + k]),
						_mm256_load_ps(&c.matrix[i * b.columns + k])
					));
			}

			for (; k < b.columns; k++) {
				c.matrix[i * b.columns + k] += a.matrix[i * a.columns + k] * b.matrix[j * b.columns + k];
			}
		}
	}

	return c;
}
matrix blocked_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {
				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {
						for (size_t n = k; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c.matrix[l * b.columns + n] += a.matrix[l * a.columns + n] * b.matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}

	return c;
}
matrix blocked_simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {
				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						size_t n = k;
						for (; n + 8 <= k + BLOCK_SIZE && n + 8 < b.columns; n += 8) {
							_mm256_store_ps(&c.matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									_mm256_load_ps(&a.matrix[l * a.columns + n]),
									_mm256_load_ps(&b.matrix[m * b.columns + n]),
									_mm256_load_ps(&c.matrix[l * b.columns + n])
								));
						}

						for (; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c.matrix[l * b.columns + n] += a.matrix[l * a.columns + n] * b.matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}

	return c;
}
matrix blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {
				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						size_t n = k;
						for (; n + 16 <= k + BLOCK_SIZE && n + 16 < b.columns; n += 8) {

							_mm256_store_ps(&c.matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									_mm256_load_ps(&a.matrix[l * a.columns + n]),
									_mm256_load_ps(&b.matrix[m * b.columns + n]),
									_mm256_load_ps(&c.matrix[l * b.columns + n])
								));

							n += 8;
							_mm256_store_ps(&c.matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									_mm256_load_ps(&a.matrix[l * a.columns + n]),
									_mm256_load_ps(&b.matrix[m * b.columns + n]),
									_mm256_load_ps(&c.matrix[l * b.columns + n])
								));
						}

						for (; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c.matrix[l * b.columns + n] += a.matrix[l * a.columns + n] * b.matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}

	return c;
}
matrix divide_conquer_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	/*
	* divide a into 4 submatrices
	* divide b into 4 submatrices
	* compute dot prod among submatrices
	*/

	if (a.columns == 1) {
		c.matrix[0] = a.matrix[0] * b.matrix[0];
		return c;
	}

	int a_split = a.matrix.size() / 4;
	int b_split = b.matrix.size() / 4;
	int c_split = c.matrix.size() / 4;

	matrix result00; result00.rows = a.rows / 2; result00.columns = b.columns / 2;
	matrix result01; result01.rows = a.rows / 2; result01.columns = b.columns / 2;
	matrix result10; result10.rows = a.rows / 2; result10.columns = b.columns / 2;
	matrix result11; result11.rows = a.rows / 2; result11.columns = b.columns / 2;

	result00.matrix = std::vector<float>(c_split, 0);
	result01.matrix = std::vector<float>(c_split, 0);
	result10.matrix = std::vector<float>(c_split, 0);
	result11.matrix = std::vector<float>(c_split, 0);

	matrix a00; a00.rows = a.rows / 2; a00.columns = a.columns / 2;
	matrix a01; a01.rows = a.rows / 2; a01.columns = a.columns / 2;
	matrix a10; a10.rows = a.rows / 2; a10.columns = a.columns / 2;
	matrix a11; a11.rows = a.rows / 2; a11.columns = a.columns / 2;

	matrix b00; b00.rows = a.rows / 2; b00.columns = a.columns / 2;
	matrix b01; b01.rows = a.rows / 2; b01.columns = a.columns / 2;
	matrix b10; b10.rows = a.rows / 2; b10.columns = a.columns / 2;
	matrix b11; b11.rows = a.rows / 2; b11.columns = a.columns / 2;

	a00.matrix = std::vector<float>(a_split, 0);
	a01.matrix = std::vector<float>(a_split, 0);
	a10.matrix = std::vector<float>(a_split, 0);
	a11.matrix = std::vector<float>(a_split, 0);

	b00.matrix = std::vector<float>(b_split, 0);
	b01.matrix = std::vector<float>(b_split, 0);
	b10.matrix = std::vector<float>(b_split, 0);
	b11.matrix = std::vector<float>(b_split, 0);

	std::memcpy(&a00.matrix[0], &a.matrix[0], a_split * sizeof(float));
	std::memcpy(&a01.matrix[0], &a.matrix[a_split], a_split * sizeof(float));
	std::memcpy(&a10.matrix[0], &a.matrix[2 * a_split], a_split * sizeof(float));
	std::memcpy(&a11.matrix[0], &a.matrix[3 * a_split], a_split * sizeof(float));

	std::memcpy(&b00.matrix[0], &b.matrix[0], b_split * sizeof(float));
	std::memcpy(&b01.matrix[0], &b.matrix[b_split], b_split * sizeof(float));
	std::memcpy(&b10.matrix[0], &b.matrix[2 * b_split], b_split * sizeof(float));
	std::memcpy(&b11.matrix[0], &b.matrix[3 * b_split], b_split * sizeof(float));

	add_matrices(
		divide_conquer_dot_prod(a00, b00),
		divide_conquer_dot_prod(a01, b10),
		result00
	);

	add_matrices(
		divide_conquer_dot_prod(a00, b01),
		divide_conquer_dot_prod(a01, b11),
		result00
	);

	add_matrices(
		divide_conquer_dot_prod(a10, b00),
		divide_conquer_dot_prod(a11, b10),
		result00
	);

	add_matrices(
		divide_conquer_dot_prod(a10, b01),
		divide_conquer_dot_prod(a11, b11),
		result00
	);


	/*
	add_matrix(multiply_matrix(a00, b00),
		multiply_matrix(a01, b10),
		result_matrix_00, split_index);
	add_matrix(multiply_matrix(a00, b01),
		multiply_matrix(a01, b11),
		result_matrix_01, split_index);
	add_matrix(multiply_matrix(a10, b00),
		multiply_matrix(a11, b10),
		result_matrix_10, split_index);
	add_matrix(multiply_matrix(a10, b01),
		multiply_matrix(a11, b11),
		result_matrix_11, split_index);
		*/

	std::memcpy(&c.matrix[0], &result00.matrix[0], result00.matrix.size() * sizeof(float));
	std::memcpy(&c.matrix[c_split], &result01.matrix[0], result01.matrix.size() * sizeof(float));
	std::memcpy(&c.matrix[2 * c_split], &result10.matrix[0], result10.matrix.size() * sizeof(float));
	std::memcpy(&c.matrix[3 * c_split], &result11.matrix[0], result11.matrix.size() * sizeof(float));

	return c;
}
matrix strassens_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	/*
	* M1 = (A11 + A22) * (B11 + B22);
	* M2 = (A21 + A22) * B11;
	* M3 = A11 * (B12 - B22);
	* M4 = A22 * (B21 - B11);
	* M5 = (A11 + A12) * B22;
	* M6 = (A21 - A11) * (B11 + B12);
	* M7 = (A12 - A22) * (B21 + B22);
	*/
	const int block = 2;

	for (size_t i = 0; i < a.rows; i += block) {
		for (size_t j = 0; j < b.rows; j += block) {
			for (size_t k = 0; k < b.columns; k += block) {

			}
		}
	}

	return c;
}