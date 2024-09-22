#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <immintrin.h>
#include <numeric>
#include <Windows.h>

#define BLOCK_SIZE 64
alignas(64) float LOCAL_A[BLOCK_SIZE * BLOCK_SIZE];
alignas(64) float LOCAL_B[BLOCK_SIZE * BLOCK_SIZE];
alignas(64) float LOCAL_C[BLOCK_SIZE * BLOCK_SIZE];
#pragma omp threadprivate(LOCAL_A, LOCAL_B, LOCAL_C)

#define LINE_SIZE 128
alignas(64) float LINE_A[LINE_SIZE];
alignas(64) float LINE_B[LINE_SIZE];
alignas(64) float LINE_C[LINE_SIZE];
#pragma omp threadprivate(LINE_A, LINE_B, LINE_C)


#define RED_TEXT 4
#define GREEN_TEXT 10
#define WHITE_TEXT 7
#define BLUE_TEXT 1
#define YELLOW_TEXT 6
#define PURPLE_TEXT 13


struct matrix {
	size_t rows;
	size_t columns;
	std::vector<float> _matrix;

	matrix() : rows(0), columns(0) {};
};

std::string matrix_to_string(matrix a);
void random_init(matrix& a);

void run_test(std::string name, matrix(*dot)(const matrix& __restrict, const matrix& __restrict));

matrix bad_dot_prod(const matrix& a, const matrix& b);
matrix base_dot_prod(const matrix& a, const matrix& b);
matrix parallel_dot_prod(const matrix& a, const matrix& b);

matrix simd_dot_prod(const matrix& a, const matrix& b);
matrix parallel_simd_dot_prod(const matrix& a, const matrix& b);
matrix simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);
matrix parallel_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);

matrix parallel_omp_simd_dot_prod(const matrix& __restrict a, const matrix& __restrict b);

matrix blocked_dot_prod(const matrix& a, const matrix& b);
matrix parallel_blocked_dot_prod(const matrix& a, const matrix& b);
matrix blocked_simd_dot_prod(const matrix& a, const matrix& b);
matrix parallel_blocked_simd_dot_prod(const matrix& a, const matrix& b);
matrix blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);
matrix parallel_blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);

matrix parallel_simd_localbuffer_dot_prod(const matrix& a, const matrix& b);
matrix parallel_simd_localbuffer_blocked_dot_prod(const matrix& a, const matrix& b);

void simple_test(matrix(*dot)(const matrix, const matrix)) {
	matrix a; a.rows = 10; a.columns = 10;
	matrix b; b.rows = 10; b.columns = 10;

	a._matrix = std::vector<float>(a.rows * a.columns);
	b._matrix = std::vector<float>(b.rows * b.columns);

	for (int r = 0; r < a.rows; r++) {
		for (int c = 0; c < a.columns; c++) {
			a._matrix[r * a.columns + c] = r;
			b._matrix[r * b.columns + c] = r;
		}
	}

	std::cout << matrix_to_string(dot(a, b));
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
	//run_test("parallel_dot_prod", parallel_dot_prod);

	//run_test("simd_dot_prod", simd_dot_prod);
	run_test("parallel_simd_dot_prod", parallel_simd_dot_prod);
	//run_test("simd_ma_unrolled_dot_prod", simd_ma_unrolled_dot_prod);
	run_test("parallel_simd_ma_unrolled_dot_prod", parallel_simd_ma_unrolled_dot_prod);

	//run_test("parallel_omp_simd_dot_prod", parallel_omp_simd_dot_prod);

	//run_test("blocked_dot_prod", blocked_dot_prod);
	//run_test("parallel_blocked_dot_prod", parallel_blocked_dot_prod);
	//run_test("blocked_simd_dot_prod", blocked_simd_dot_prod);
	run_test("parallel_blocked_simd_dot_prod", parallel_blocked_simd_dot_prod);
	//run_test("blocked_simd_ma_unrolled_dot_prod", blocked_simd_ma_unrolled_dot_prod);
	run_test("parallel_blocked_simd_ma_unrolled_dot_prod", parallel_blocked_simd_ma_unrolled_dot_prod);

	//run_test("parallel_simd_localbuffer_dot_prod", parallel_simd_localbuffer_dot_prod);
	run_test("parallel_simd_localbuffer_blocked_dot_prod", parallel_simd_localbuffer_blocked_dot_prod);

	return 0;
}

std::string matrix_to_string(matrix a) {
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

	a._matrix = std::vector<float>(a.rows * a.columns);
	for (int i = 0; i < a.rows * a.columns; i++) {
		a._matrix[i] = dist(gen);
	}
}

void run_test(std::string name, matrix(*dot)(const matrix& __restrict, const matrix& __restrict)) {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	const int max_size = 1024;
	const int multiplier = 2;
	int runs = 1024;

	matrix c;

	SetConsoleTextAttribute(hConsole, YELLOW_TEXT);
	std::cout << "\n" << name << ":\n";
	for (int size = 8; size <= max_size; size *= multiplier, runs /= multiplier) {
		runs = runs > 8 ? runs : 8;

		matrix a; a.rows = size; a.columns = size;
		matrix b; b.rows = size; b.columns = size;

		random_init(a);
		random_init(b);

		std::vector<double> best(runs);

		// warmup runs
		c = (*dot)(a, b);

		for (int i = 0; i < runs; i++) {
			auto start = std::chrono::high_resolution_clock::now();
			c = (*dot)(a, b);
			best[i] = (std::chrono::high_resolution_clock::now() - start).count() / 1000000.00;
		}

		double min = *std::min_element(&best[0], &best[runs]);
		double max = *std::max_element(&best[0], &best[runs]); 
		double sum = std::accumulate(&best[0], &best[runs], 0.0);
		 
		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\t" << size << "x" << size << ": ";
		SetConsoleTextAttribute(hConsole, GREEN_TEXT); std::cout << min << "ms";  
		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " - ";
		SetConsoleTextAttribute(hConsole, RED_TEXT); std::cout << max << "ms";
		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " :: ";
		SetConsoleTextAttribute(hConsole, BLUE_TEXT); std::cout << (double)(sum / runs) << "ms";
		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " taken over ";
		SetConsoleTextAttribute(hConsole, YELLOW_TEXT); std::cout << runs;
		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " runs\n";

		SetConsoleTextAttribute(hConsole, WHITE_TEXT);
	}
}

matrix bad_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.columns; j++) {
			for (size_t k = 0; k < b.rows; k++) {
				c._matrix[i * b.columns + j] += a._matrix[i * a.columns + k] * b._matrix[k * b.columns + j];
			}
		}
	}

	return c;
}
matrix base_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			for (size_t k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}

	return c;
}
matrix parallel_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {
			for (int k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}

	return c;
}

matrix simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix parallel_simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix parallel_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}

matrix parallel_omp_simd_dot_prod(const matrix& __restrict a, const matrix& __restrict b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for 
	for (int i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {

			#pragma omp simd
			for (size_t k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}

	return c;
}

matrix blocked_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix parallel_blocked_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix blocked_simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix parallel_blocked_simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}
matrix parallel_blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}

matrix parallel_simd_localbuffer_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

	alignas(64) std::vector<float> local_a(a.rows * a.columns);
	alignas(64) std::vector<float> local_b(b.rows * b.columns);
	alignas(64) std::vector<float> local_c(c.rows * c.columns, 0);

	std::copy(a._matrix.begin(), a._matrix.end(), local_a.begin());
	std::copy(b._matrix.begin(), b._matrix.end(), local_b.begin());

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

	std::copy(local_c.begin(), local_c.end(), c._matrix.begin());

	return c;
}
matrix parallel_simd_localbuffer_blocked_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}

matrix strassens_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c._matrix = std::vector<float>(a.rows * b.columns, 0);

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

	return c;
}