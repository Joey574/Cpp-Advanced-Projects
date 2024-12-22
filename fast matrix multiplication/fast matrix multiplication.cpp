#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <immintrin.h>
#include <numeric>
#include <Windows.h>
#include <iomanip>

#include "cuda_implementations.cuh"

#define BLOCK_SIZE 64
alignas(64) float LOCAL_A[BLOCK_SIZE * BLOCK_SIZE];
alignas(64) float LOCAL_B[BLOCK_SIZE * BLOCK_SIZE];
alignas(64) float LOCAL_C[BLOCK_SIZE * BLOCK_SIZE];
#pragma omp threadprivate(LOCAL_A, LOCAL_B, LOCAL_C)

#define RED_TEXT 4
#define GREEN_TEXT 10
#define WHITE_TEXT 7
#define BLUE_TEXT 1
#define YELLOW_TEXT 6
#define PURPLE_TEXT 13

struct matrix {
	size_t rows;
	size_t columns;
	float* _matrix;

	matrix() : rows(0), columns(0), _matrix(nullptr) {}
	matrix(size_t r, size_t c) : rows(r), columns(c), _matrix((float*)_aligned_malloc(r * c * sizeof(float), 64)) {}

	~matrix() { if (_matrix) { _aligned_free(_matrix); } }
};

std::string matrix_to_string(const matrix& a);
void random_init(matrix& a);

void run_test(std::string name, void(*dot)(const matrix&, const matrix&, matrix&));
void test_results(int size, int runs, int size_f_len, double min, double max, double sum, const HANDLE& hConsole);

void bad_dot_prod(const matrix& a, const matrix& b, matrix& c);
void base_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_dot_prod(const matrix& a, const matrix& b, matrix& c);

void simd_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_simd_dot_prod(const matrix& a, const matrix& b, matrix& c);
void simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c);

void parallel_omp_simd_dot_prod(const matrix& a, const matrix& b, matrix& c);

void blocked_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_blocked_dot_prod(const matrix& a, const matrix& b, matrix& c);
void blocked_simd_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_blocked_simd_dot_prod(const matrix& a, const matrix& b, matrix& c);
void blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c);

void parallel_simd_localbuffer_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_simd_localbuffer_blocked_dot_prod(const matrix& a, const matrix& b, matrix& c);

void cuda_dot_prod(const matrix& a, const matrix& b, matrix& c);


void simple_test(matrix(*dot)(const matrix, const matrix)) {
	matrix a(10, 10);
	matrix b(10, 10);

	for (int r = 0; r < a.rows; r++) {
		for (int c = 0; c < a.columns; c++) {
			a._matrix[r * a.columns + c] = r;
			b._matrix[r * b.columns + c] = r;
		}
	}

	std::cout << matrix_to_string(dot(a, b)) << "\n";
}
void vector_test(const std::vector<float>& __restrict a, const std::vector<float>& __restrict b, std::vector<float>& __restrict c) {

	#pragma omp simd
	for (size_t i = 0; i < a.size(); i++) {
		c[i] = a[i] + b[i];
	}
}

std::vector<float> vector_test_2(std::vector<float> a, std::vector<float> b) {
	std::vector<float> c(a.size(), 0);

	#pragma omp simd
	for (size_t i = 0; i < a.size(); i++) {
		c[i] += a[i] + b[i];
	}

	return c;
}

int main()
{
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	/*std::vector<float> a(64, 2);
	std::vector<float> b(64, 5);
	std::vector<float> c(64);

	vector_test(a, b, c);
	vector_test_2(a, b);

	return 0;*/

	//run_test("bad_dot_prod", bad_dot_prod);
	//run_test("base_dot_prod", base_dot_prod);
	run_test("parallel_dot_prod", parallel_dot_prod);

	run_test("simd_dot_prod", simd_dot_prod);
	run_test("parallel_simd_dot_prod", parallel_simd_dot_prod);
	//run_test("simd_ma_unrolled_dot_prod", simd_ma_unrolled_dot_prod);
	//run_test("parallel_simd_ma_unrolled_dot_prod", parallel_simd_ma_unrolled_dot_prod);

	run_test("parallel_omp_simd_dot_prod", parallel_omp_simd_dot_prod);

	//run_test("blocked_dot_prod", blocked_dot_prod);
	//run_test("parallel_blocked_dot_prod", parallel_blocked_dot_prod);
	//run_test("blocked_simd_dot_prod", blocked_simd_dot_prod);
	run_test("parallel_blocked_simd_dot_prod", parallel_blocked_simd_dot_prod);
	//run_test("blocked_simd_ma_unrolled_dot_prod", blocked_simd_ma_unrolled_dot_prod);
	//run_test("parallel_blocked_simd_ma_unrolled_dot_prod", parallel_blocked_simd_ma_unrolled_dot_prod);

	//run_test("parallel_simd_localbuffer_dot_prod", parallel_simd_localbuffer_dot_prod);
	run_test("parallel_simd_localbuffer_blocked_dot_prod", parallel_simd_localbuffer_blocked_dot_prod);

	run_test("cuda_dot_prod", cuda_dot_prod);

	return 0;
}

std::string matrix_to_string(const matrix& a) {
	std::string out = "";

	for (int r = 0; r < a.rows; r++) {
		for (int c = 0; c < a.columns; c++) {
			out.append(std::to_string(a._matrix[r * a.columns + c])).append(" ");
		}
		out.append("\n");
	}

	return out;
}
void random_init(matrix& a) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

	for (int i = 0; i < a.rows * a.columns; i++) {
		a._matrix[i] = dist(gen);
	}
}

void run_test(std::string name, void(*dot)(const matrix&, const matrix&, matrix&)) {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	const int max_size = 1024;
	const int multiplier = 2;

	const size_t max_runs = 1024;
	size_t runs = max_runs;
	double best[max_runs] = {};

	const int size_format_length = (std::to_string(max_size).length() * 2) + 2;


	SetConsoleTextAttribute(hConsole, YELLOW_TEXT);
	std::cout << "\n" << name << ":\n";
	for (size_t size = 8; size <= max_size; size *= multiplier, runs /= multiplier) {
		runs = runs > 8 ? runs : 8;

		matrix a(size, size);
		matrix b(size, size);
		matrix c(size, size);

		random_init(a);
		random_init(b);


		// warmup runs
		(*dot)(a, b, c);
		(*dot)(a, b, c);
		(*dot)(a, b, c);

		for (size_t i = 0; i < runs; i++) {
			auto start = std::chrono::high_resolution_clock::now();
			(*dot)(a, b, c);
			best[i] = (std::chrono::high_resolution_clock::now() - start).count() / 1000000.00;
		}

		double min = *std::min_element(&best[0], &best[runs]);
		double max = *std::max_element(&best[0], &best[runs]); 
		double sum = std::accumulate(&best[0], &best[runs], 0.0);

		test_results(size, runs, size_format_length, min, max, sum, hConsole);
	}
}
void test_results(int size, int runs, int size_f_len, double min, double max, double sum, const HANDLE& hConsole) {
	std::string formatted_size = std::to_string(size).append("x").append(std::to_string(size)).append(":"); formatted_size.resize(size_f_len, ' ');
	std::string formatted_min = std::to_string(min); formatted_min.resize(7, ' '); formatted_min.append("ms");
	std::string formatted_max = std::to_string(max); formatted_max.resize(7, ' '); formatted_max.append("ms");
	std::string formatted_avg = std::to_string(sum / runs); formatted_avg.resize(7, ' '); formatted_avg.append("ms");

	SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\t" << formatted_size << "\t";
	SetConsoleTextAttribute(hConsole, GREEN_TEXT); std::cout << formatted_min;
	SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " - ";
	SetConsoleTextAttribute(hConsole, RED_TEXT); std::cout << formatted_max;
	SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " :: ";
	SetConsoleTextAttribute(hConsole, BLUE_TEXT); std::cout << formatted_avg;
	SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\ttaken over ";
	SetConsoleTextAttribute(hConsole, YELLOW_TEXT); std::cout << runs;
	SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " runs\n";
}

void bad_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.columns; j++) {
			for (size_t k = 0; k < b.rows; k++) {
				c._matrix[i * b.columns + j] += a._matrix[i * a.columns + k] * b._matrix[k * b.columns + j];
			}
		}
	}
}
void base_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			for (size_t k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void parallel_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {
			for (int k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}

void simd_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			__m256 scalar = _mm256_set1_ps(a._matrix[i * a.columns + j]);

			size_t k = 0;
			for (; k + 8 <= b.columns; k += 8) {
				_mm256_store_ps(&c._matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar,
						_mm256_load_ps(&b._matrix[j * b.columns + k]),
						_mm256_load_ps(&c._matrix[i * b.columns + k])
						));
			}

			for (; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void parallel_simd_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {

			__m256 scalar = _mm256_set1_ps(a._matrix[i * a.columns + j]);

			int k = 0;
			for (; k + 8 <= b.columns; k += 8) {
				_mm256_store_ps(&c._matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar ,
						_mm256_load_ps(&b._matrix[j * b.columns + k]),
						_mm256_load_ps(&c._matrix[i * b.columns + k])
					));
			}

			for (; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			__m256 scalar = _mm256_set1_ps(a._matrix[i * a.columns + j]);

			size_t k = 0;
			for (; k + 16 <= b.columns; k += 8) {
				_mm256_store_ps(&c._matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar,
						_mm256_load_ps(&b._matrix[j * b.columns + k]),
						_mm256_load_ps(&c._matrix[i * b.columns + k])
					));

				k += 8;
				_mm256_store_ps(&c._matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar,
						_mm256_load_ps(&b._matrix[j * b.columns + k]),
						_mm256_load_ps(&c._matrix[i * b.columns + k])
					));
			}

			for (; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void parallel_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {

			__m256 scalar = _mm256_set1_ps(a._matrix[i * a.columns + j]);

			int k = 0;
			for (; k + 16 <= b.columns; k += 8) {
				_mm256_store_ps(&c._matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar,
						_mm256_load_ps(&b._matrix[j * b.columns + k]),
						_mm256_load_ps(&c._matrix[i * b.columns + k])
					));

				k += 8;
				_mm256_store_ps(&c._matrix[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar,
						_mm256_load_ps(&b._matrix[j * b.columns + k]),
						_mm256_load_ps(&c._matrix[i * b.columns + k])
					));
			}

			for (; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}

void parallel_omp_simd_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for 
	for (int i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			#pragma omp simd
			for (size_t k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}

void blocked_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {
				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {
						for (size_t n = k; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c._matrix[l * b.columns + n] += a._matrix[l * a.columns + m] * b._matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}
}
void parallel_blocked_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {
				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {
						for (size_t n = k; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c._matrix[l * b.columns + n] += a._matrix[l * a.columns + m] * b._matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}
}
void blocked_simd_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						__m256 scalar = _mm256_set1_ps(a._matrix[l * a.columns + m]);

						size_t n = k;
						for (; n + 8 <= k + BLOCK_SIZE && n + 8 <= b.columns; n += 8) {
							_mm256_store_ps(&c._matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&b._matrix[m * b.columns + n]),
									_mm256_load_ps(&c._matrix[l * b.columns + n])
								));
						}

						for (; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c._matrix[l * b.columns + n] += a._matrix[l * a.columns + m] * b._matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}
}
void parallel_blocked_simd_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						__m256 scalar = _mm256_set1_ps(a._matrix[l * a.columns + m]);

						size_t n = k;
						for (; n + 8 <= k + BLOCK_SIZE && n + 8 <= b.columns; n += 8) {
							_mm256_store_ps(&c._matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&b._matrix[m * b.columns + n]),
									_mm256_load_ps(&c._matrix[l * b.columns + n])
								));
						}

						for (; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c._matrix[l * b.columns + n] += a._matrix[l * a.columns + m] * b._matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}
}
void blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	for (size_t i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						__m256 scalar = _mm256_set1_ps(a._matrix[l * a.columns + m]);

						size_t n = k;
						for (; n + 16 <= k + BLOCK_SIZE && n + 16 <= b.columns; n += 8) {

							_mm256_store_ps(&c._matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&b._matrix[m * b.columns + n]),
									_mm256_load_ps(&c._matrix[l * b.columns + n])
								));

							n += 8;
							_mm256_store_ps(&c._matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&b._matrix[m * b.columns + n]),
									_mm256_load_ps(&c._matrix[l * b.columns + n])
								));
						}

						for (; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c._matrix[l * b.columns + n] += a._matrix[l * a.columns + m] * b._matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}
}
void parallel_blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (size_t l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (size_t m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						__m256 scalar = _mm256_set1_ps(a._matrix[l * a.columns + m]);

						size_t n = k;
						for (; n + 16 <= k + BLOCK_SIZE && n + 16 <= b.columns; n += 8) {

							_mm256_store_ps(&c._matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&b._matrix[m * b.columns + n]),
									_mm256_load_ps(&c._matrix[l * b.columns + n])
								));

							n += 8;
							_mm256_store_ps(&c._matrix[l * b.columns + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&b._matrix[m * b.columns + n]),
									_mm256_load_ps(&c._matrix[l * b.columns + n])
								));
						}

						for (; n < k + BLOCK_SIZE && n < b.columns; n++) {
							c._matrix[l * b.columns + n] += a._matrix[l * a.columns + m] * b._matrix[m * b.columns + n];
						}
					}
				}
			}
		}
	}
}

void parallel_simd_localbuffer_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	alignas(64) std::vector<float> local_a(a.rows * a.columns);
	alignas(64) std::vector<float> local_b(b.rows * b.columns);
	alignas(64) std::vector<float> local_c(c.rows * c.columns, 0);

	std::copy(&a._matrix[0], &a._matrix[a.rows * a.columns], local_a.begin());
	std::copy(&b._matrix[0], &b._matrix[b.rows * b.columns], local_b.begin());

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			__m256 scalar = _mm256_set1_ps(local_a[i * a.columns + j]);

			size_t k = 0;
			for (; k + 8 <= b.columns; k += 8) {
				_mm256_store_ps(&local_c[i * b.columns + k],
					_mm256_fmadd_ps(
						scalar,
						_mm256_load_ps(&local_b[j * b.columns + k]),
						_mm256_load_ps(&local_c[i * b.columns + k])
				));
			}

			for (; k < b.columns; k++) {
				local_c[i * b.columns + k] += local_a[i * a.columns + j] * local_b[j * b.columns + k];
			}
		}
	}

	std::copy(local_c.begin(), local_c.end(), &c._matrix[0]);
}
void parallel_simd_localbuffer_blocked_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (size_t j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (size_t k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (size_t a_r = 0; a_r < BLOCK_SIZE; a_r++) {
					#pragma omp simd
					for (size_t a_c = 0; a_c < BLOCK_SIZE; a_c++) {
						LOCAL_A[a_r * BLOCK_SIZE + a_c] = a_r + i >= a.rows || a_c + j >= a.columns ? 0.0f : a._matrix[(a_r + i) * a.columns + (a_c + j)];
					}
				}

				for (size_t b_r = 0; b_r < BLOCK_SIZE; b_r++) {
					#pragma omp simd
					for (size_t b_c = 0; b_c < BLOCK_SIZE; b_c++) {
						LOCAL_B[b_r * BLOCK_SIZE + b_c] = b_r + j >= b.rows || b_c + k >= b.columns ? 0.0f : b._matrix[(b_r + j) * b.columns + (b_c + k)];
					}
				}
				std::fill(&LOCAL_C[0], &LOCAL_C[BLOCK_SIZE * BLOCK_SIZE], 0);


				for (size_t l = 0; l < BLOCK_SIZE; l++) {
					for (size_t m = 0; m < BLOCK_SIZE; m++) {

						__m256 scalar = _mm256_set1_ps(LOCAL_A[l * BLOCK_SIZE + m]);

						size_t n = 0;
						for (; n + 8 <= BLOCK_SIZE; n += 8) {
							_mm256_store_ps(&LOCAL_C[l * BLOCK_SIZE + n],
								_mm256_fmadd_ps(
									scalar,
									_mm256_load_ps(&LOCAL_B[m * BLOCK_SIZE + n]),
									_mm256_load_ps(&LOCAL_C[l * BLOCK_SIZE + n])
								));
						}

						for (; n < BLOCK_SIZE; n++) {
							LOCAL_C[l * BLOCK_SIZE + n] += LOCAL_A[l * BLOCK_SIZE + m] * LOCAL_B[m * BLOCK_SIZE + n];
						}
					}
				}

				// copy into c_matrix
				for (int c_r = 0; c_r < BLOCK_SIZE && c_r + i < c.rows; c_r++) {
					for (int c_c = 0; c_c < BLOCK_SIZE && c_c + k < c.columns; c_c++) {
						c._matrix[(c_r + i) * c.columns + (c_c + k)] = LOCAL_C[c_r * BLOCK_SIZE + c_c];
					}
				}
			}
		}
	}
}

void cuda_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	float* d_a;
	float* d_b;
	float* d_c;

	cudaMalloc(&d_a, a.rows * a.columns * sizeof(float));
	cudaMalloc(&d_b, b.rows * b.columns * sizeof(float));

	cudaMalloc(&d_c, a.rows * b.columns * sizeof(float));

	cudaMemcpy(d_a, &a._matrix[0], a.rows * a.columns * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, &b._matrix[0], b.rows * b.columns * sizeof(float), cudaMemcpyHostToDevice);

	dim3 dim_block(8);
	dim3 dim_grid(ceil((a.rows * b.columns) / 8.0f));

	call_kernal(d_a, d_b, d_c, a.rows, dim_grid, dim_block);

	cudaMemcpy(&c._matrix[0], d_c, c.rows * c.columns * sizeof(float), cudaMemcpyDeviceToHost);

	cudaFree(d_a);
	cudaFree(d_b);
	cudaFree(d_c);
}

void strassens_dot_prod(const matrix& a, const matrix& b, matrix& c) {

	/*
	* M1 = (A11 + A22) * (B11 + B22);
	* M2 = (A21 + A22) * B11;
	* M3 = A11 * (B12 - B22);
	* M4 = A22 * (B21 - B11);
	* M5 = (A11 + A12) * B22;
	* M6 = (A21 - A11) * (B11 + B12);
	* M7 = (A12 - A22) * (B21 + B22);
	* 
	*   _   _       _   _ 
	*  | a b |  *  | e f |  =  
	*  | c d |     | g h |
	*   -   -       -   -
	*/
	const int block = 2;

	for (size_t i = 0; i < a.rows; i += block) {
		for (size_t j = 0; j < b.rows; j += block) {
			for (size_t k = 0; k < b.columns; k += block) {

			}
		}
	}
}