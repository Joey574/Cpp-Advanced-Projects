#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <immintrin.h>
#include <numeric>
#include <Windows.h>

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

void random_init(matrix& a);

void run_test(std::string name, void(*dot)(const matrix&, const matrix&, matrix&));

void base_dot_prod(const matrix& a, const matrix& b, matrix& c);
void base_dot_prod_t_a(const matrix& a, const matrix& b, matrix& c);
void base_dot_prod_t_b(const matrix& a, const matrix& b, matrix& c);

void parallel_simd_dot_prod(const matrix& a, const matrix& b, matrix& c);
void parallel_simd_dot_prod_t_a(const matrix& a, const matrix& b, matrix& c);
void parallel_simd_dot_prod_t_b(const matrix& a, const matrix& b, matrix& c);

void parallel_transpose_simd_dot_prod_t_b(const matrix& a, const matrix& b, matrix& c);

int main()
{
	SetPriorityClass(GetStdHandle, REALTIME_PRIORITY_CLASS);

	std::cout << "Base Dot Prods\n";
	//run_test("base_dot_prod", base_dot_prod);
	run_test("parallel_simd_dot_prod", parallel_simd_dot_prod);


	std::cout << "\na.T Dot Prods\n";
	//run_test("base_dot_prod_t_a", base_dot_prod_t_a);
	run_test("parallel_simd_dot_prod_t_a", parallel_simd_dot_prod_t_a);


	std::cout << "\nb.T Dot Prods\n";
	//run_test("base_dot_prod_t_b", base_dot_prod_t_b);
	run_test("parallel_simd_dot_prod_t_b", parallel_simd_dot_prod_t_b);
	run_test("parallel_transpose_simd_dot_prod_t_b", parallel_transpose_simd_dot_prod_t_b);
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
void run_test(std::string name, void(*dot)(const matrix&, const matrix&, matrix&)) {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	const int max_size = 2048;
	const int multiplier = 2;
	int runs = 1024;

	SetConsoleTextAttribute(hConsole, YELLOW_TEXT);
	std::cout << name << ":\n";
	for (int size = 256; size <= max_size; size *= multiplier, runs /= multiplier) {
		runs = runs > 8 ? runs : 8;

		matrix a; a.rows = size; a.columns = size;
		matrix b; b.rows = size; b.columns = size;
		matrix c; c.rows = size; c.columns = size; c._matrix = std::vector<float>(size * size, 0);

		random_init(a);
		random_init(b);

		std::vector<double> best(runs);

		// warmup runs
		(*dot)(a, b, c);

		for (int i = 0; i < runs; i++) {

			std::fill(c._matrix.begin(), c._matrix.end(), 0);

			auto start = std::chrono::high_resolution_clock::now();
			(*dot)(a, b, c);
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


void base_dot_prod(const matrix& a, const matrix& b, matrix& c) {
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			for (size_t k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void base_dot_prod_t_a(const matrix& a, const matrix& b, matrix& c) {
	for (size_t i = 0; i < a.columns; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			for (size_t k = 0; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[j * a.columns + i] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void base_dot_prod_t_b(const matrix& a, const matrix& b, matrix& c) {
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t k = 0; k < b.rows; k++) {
			for (size_t j = 0; j < b.columns; j++) {
				c._matrix[i * b.rows + k] += a._matrix[i * a.columns + j] * b._matrix[k * b.columns + j];
			}
		}
	}
}

void parallel_simd_dot_prod(const matrix& a, const matrix& b, matrix& c) {
	#pragma omp parallel for
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			size_t k = 0; 

			const __m256 _a = _mm256_load_ps(&a._matrix[i * a.columns + j]);
			for (; k + 8 <= b.columns; k += 8) {
				const __m256 _b = _mm256_load_ps(&b._matrix[j * b.columns + k]);
				const __m256 _c = _mm256_load_ps(&c._matrix[i * b.columns + k]);
				const __m256 _res = _mm256_fmadd_ps(_a, _b, _c);

				_mm256_store_ps(&c._matrix[i * b.columns + k], _res);
			}

			for (; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void parallel_simd_dot_prod_t_a(const matrix& a, const matrix& b, matrix& c) {
	#pragma omp parallel for
	for (size_t i = 0; i < a.columns; i++) {
		for (size_t j = 0; j < b.rows; j++) {
			size_t k = 0;

			const __m256 _a = _mm256_load_ps(&a._matrix[j * a.columns + i]);
			for (; k + 8 <= b.columns; k += 8) {
				const __m256 _b = _mm256_load_ps(&b._matrix[j * b.columns + k]);
				const __m256 _c = _mm256_load_ps(&c._matrix[i * b.columns + k]);
				const __m256 _res = _mm256_fmadd_ps(_a, _b, _c);

				_mm256_store_ps(&c._matrix[i * b.columns + k], _res);
			}

			for (; k < b.columns; k++) {
				c._matrix[i * b.columns + k] += a._matrix[i * a.columns + j] * b._matrix[j * b.columns + k];
			}
		}
	}
}
void parallel_simd_dot_prod_t_b(const matrix& a, const matrix& b, matrix& c) {
	#pragma omp parallel for
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t k = 0; k < b.rows; k++) {
			size_t j = 0;
			
			__m256 sum = _mm256_setzero_ps();
			for (; j + 8 <= b.columns; j += 8) {
				const __m256 _a = _mm256_load_ps(&a._matrix[i * a.columns + j]);
				const __m256 _b = _mm256_load_ps(&b._matrix[k * b.columns + j]);

				sum = _mm256_fmadd_ps(_a, _b, sum);
			}

			// sum values into one location
			const __m128 hi_four = _mm256_extractf128_ps(sum, 1);
			const __m128 lo_four = _mm256_extractf128_ps(sum, 0);
			const __m128 sum_four = _mm_add_ps(lo_four, hi_four);

			const __m128 lo_dual = sum_four;
			const __m128 hi_dual = _mm_movehl_ps(lo_dual, sum_four);
			const __m128 sum_dual = _mm_add_ps(lo_dual, hi_dual);

			const __m128 lo = sum_dual;
			const __m128 hi = _mm_shuffle_ps(sum_dual, sum_dual, 0x1);
			const __m128 fsum = _mm_add_ss(lo, hi);

			c._matrix[i * b.rows + k] += _mm_cvtss_f32(fsum);

			for (; j < b.columns; j++) {
				c._matrix[i * b.rows + k] += a._matrix[i * a.columns + j] * b._matrix[k * b.columns + j];
			}
		}
	}
}

void parallel_transpose_simd_dot_prod_t_b(const matrix& a, const matrix& b, matrix& c) {
	matrix b_t; b_t.rows = b.columns; b_t.columns = b.rows; b_t._matrix = std::vector<float>(b.rows * b.columns, 0);

	#pragma omp parallel for
	for (size_t r = 0; r < b.rows; r++) {
		for (size_t c = 0; c < b.columns; c++) {
			b_t._matrix[c * b_t.columns + r] = b._matrix[r * b.columns + c];
		}
	}


	#pragma omp parallel for
	for (size_t i = 0; i < a.rows; i++) {
		for (size_t j = 0; j < b_t.rows; j++) {
			size_t k = 0;

			const __m256 _a = _mm256_load_ps(&a._matrix[i * a.columns + j]);
			for (; k + 8 <= b_t.columns; k += 8) {
				const __m256 _b = _mm256_load_ps(&b_t._matrix[j * b.columns + k]);
				const __m256 _c = _mm256_load_ps(&c._matrix[i * b.columns + k]);
				const __m256 _res = _mm256_fmadd_ps(_a, _b, _c);

				_mm256_store_ps(&c._matrix[i * b_t.columns + k], _res);
			}

			for (; k < b_t.columns; k++) {
				c._matrix[i * b_t.columns + k] += a._matrix[i * a.columns + j] * b_t._matrix[j * b_t.columns + k];
			}
		}
	}
}