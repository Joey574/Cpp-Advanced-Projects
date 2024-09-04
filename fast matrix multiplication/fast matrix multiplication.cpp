#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <immintrin.h>
#include <omp.h>
#include <Windows.h>

#define BLOCK_SIZE 64

#define RED_TEXT 4
#define GREEN_TEXT 10
#define WHITE_TEXT 7
#define BLUE_TEXT 1
#define YELLOW_TEXT 6
#define PURPLE_TEXT 13

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
matrix parallel_simd_dot_prod(const matrix& a, const matrix& b);
matrix simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);
matrix parallel_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);

matrix blocked_dot_prod(const matrix& a, const matrix& b);
matrix parallel_blocked_dot_prod(const matrix& a, const matrix& b);
matrix blocked_simd_dot_prod(const matrix& a, const matrix& b);
matrix parallel_blocked_simd_dot_prod(const matrix& a, const matrix& b);
matrix blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);
matrix parallel_blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b);

int main()
{
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	run_test("bad_dot_prod", bad_dot_prod);
	run_test("base_dot_prod", base_dot_prod);
	run_test("parallel_dot_prod", parallel_dot_prod);

	run_test("simd_dot_prod", simd_dot_prod);
	run_test("parallel_simd_dot_prod", parallel_simd_dot_prod);
	run_test("simd_ma_unrolled_dot_prod", simd_ma_unrolled_dot_prod);
	run_test("parallel_simd_ma_unrolled_dot_prod", parallel_simd_ma_unrolled_dot_prod);

	run_test("blocked_dot_prod", blocked_dot_prod);
	run_test("parallel_blocked_dot_prod", parallel_blocked_dot_prod);
	run_test("blocked_simd_dot_prod", blocked_simd_dot_prod);
	run_test("parallel_blocked_simd_dot_prod", parallel_blocked_simd_dot_prod);
	run_test("blocked_simd_ma_unrolled_dot_prod", blocked_simd_ma_unrolled_dot_prod);
	run_test("parallel_blocked_simd_ma_unrolled_dot_prod", parallel_blocked_simd_ma_unrolled_dot_prod);

	return 0;
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

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	const int max_size = 1024;
	const int multiplier = 2;
	const int runs = 10;

	matrix c;

	SetConsoleTextAttribute(hConsole, YELLOW_TEXT);
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
			best[i] = (std::chrono::high_resolution_clock::now() - start).count();
		}

		double min = *std::min_element(&best[0], &best[runs]);
		double max = *std::max_element(&best[0], &best[runs]);

		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\t" << size << "x" << size << ": ";
		SetConsoleTextAttribute(hConsole, GREEN_TEXT); std::cout << (min / 1000000.00) << "ms";  
		SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " - ";
		SetConsoleTextAttribute(hConsole, RED_TEXT); std::cout << (max / 1000000.00) << "ms\n";

		SetConsoleTextAttribute(hConsole, WHITE_TEXT);
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
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {
			for (int k = 0; k < b.columns; k++) {
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
			for (; k + 8 <= b.columns; k += 8) {
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
matrix parallel_simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {

			int k = 0;
			for (; k + 8 <= b.columns; k += 8) {
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
			for (; k + 16 <= b.columns; k += 8) {
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
matrix parallel_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.rows; j++) {

			int k = 0;
			for (; k + 16 <= b.columns; k += 8) {
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
matrix parallel_blocked_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (int j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (int k = 0; k < b.columns; k += BLOCK_SIZE) {
				for (int l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (int m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {
						for (int n = k; n < k + BLOCK_SIZE && n < b.columns; n++) {
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
						for (; n + 8 <= k + BLOCK_SIZE && n + 8 <= b.columns; n += 8) {
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
matrix parallel_blocked_simd_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (int j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (int k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (int l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (int m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						int n = k;
						for (; n + 8 <= k + BLOCK_SIZE && n + 8 <= b.columns; n += 8) {
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
						for (; n + 16 <= k + BLOCK_SIZE && n + 16 <= b.columns; n += 8) {

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
matrix parallel_blocked_simd_ma_unrolled_dot_prod(const matrix& a, const matrix& b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(a.rows * b.columns, 0);

	#pragma omp parallel for
	for (int i = 0; i < a.rows; i += BLOCK_SIZE) {
		for (int j = 0; j < b.rows; j += BLOCK_SIZE) {
			for (int k = 0; k < b.columns; k += BLOCK_SIZE) {

				for (int l = i; l < i + BLOCK_SIZE && l < a.rows; l++) {
					for (int m = j; m < j + BLOCK_SIZE && m < b.rows; m++) {

						int n = k;
						for (; n + 16 <= k + BLOCK_SIZE && n + 16 <= b.columns; n += 8) {

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