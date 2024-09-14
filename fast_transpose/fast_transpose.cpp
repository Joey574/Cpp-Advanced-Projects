#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <Windows.h>
#include <random>
#include <immintrin.h>
#include <omp.h>

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

void run_test(std::string name, void(*transpose)(const matrix&, matrix&));

void flipped_transpose(const matrix& mat, matrix& mat_t);
void basic_transpose(const matrix& mat, matrix& mat_t);

void parallel_flipped_transpose(const matrix& mat, matrix& mat_t);
void parallel_transpose(const matrix& mat, matrix& mat_t);

void parallel_flipped_unrolled_transpose(const matrix& mat, matrix& mat_t);

void parallel_flipped_simd_transpose(const matrix& mat, matrix& mat_t);


void random_init(matrix& a) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

    a._matrix = std::vector<float>(a.rows * a.columns);
    for (size_t i = 0; i < a.rows * a.columns; i++) {
        a._matrix[i] = dist(gen);
    }
}


int main()
{
    run_test("flipped_transpose", flipped_transpose);
    run_test("basic_transpose", basic_transpose);

    run_test("parallel_flipped_transpose", parallel_flipped_transpose);
    run_test("parallel_transpose", parallel_transpose);

    run_test("parallel_flipped_unrolled_transpose", parallel_flipped_unrolled_transpose);

    run_test("parallel_flipped_simd_transpose", parallel_flipped_simd_transpose);
}

void run_test(std::string name, void(*transpose)(const matrix&, matrix&)) {

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    const int max_size = 4096;
    const int multiplier = 2;
    const int runs = 10;

    SetConsoleTextAttribute(hConsole, YELLOW_TEXT); std::cout << name << ":\n";

    for (int size = 512; size <= max_size; size *= multiplier) {

        double best[runs];

        matrix mat; mat.rows = size; mat.columns = size;
        random_init(mat);

        matrix mat_t; mat_t.rows = mat.columns; mat_t.columns = mat.rows;
        mat_t._matrix = std::vector<float>(mat_t.rows * mat_t.columns);

        for (int r = 0; r < runs; r++) {
            auto start = std::chrono::high_resolution_clock::now();
            transpose(mat, mat_t);
            best[r] = (std::chrono::high_resolution_clock::now() - start).count();

            for (size_t i = 0; i < mat.rows; i++) {
                for (size_t c = 0; c < mat.columns; c++) {
                    if (mat._matrix[i * mat.columns + c] != mat_t._matrix[c * mat_t.columns + i]) {
                        std::cout << name << ": failed\n";
                        goto failed;
                    }
                }
            }
        }
        failed:

        double min = *std::min_element(&best[0], &best[runs]);
        double max = *std::max_element(&best[0], &best[runs]);

        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\t" << size << " x " << size << ": ";
        SetConsoleTextAttribute(hConsole, GREEN_TEXT); std::cout << (min / 1000000.00) << "ms";
        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " - ";
        SetConsoleTextAttribute(hConsole, RED_TEXT); std::cout << (max / 1000000.00) << "ms\n";

        SetConsoleTextAttribute(hConsole, WHITE_TEXT);
    }
}


void flipped_transpose(const matrix& mat, matrix& mat_t) {
    for (size_t c = 0; c < mat.columns; c++) {
        for (size_t r = 0; r < mat.rows; r++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}
void basic_transpose(const matrix& mat, matrix& mat_t) {
    for (size_t r = 0; r < mat.rows; r++) {
        for (size_t c = 0; c < mat.columns; c++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}

void parallel_flipped_transpose(const matrix& mat, matrix& mat_t) {

    #pragma omp parallel for
    for (int c = 0; c < mat.columns; c++) {
        for (size_t r = 0; r < mat.rows; r++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}
void parallel_transpose(const matrix& mat, matrix& mat_t) {

    #pragma omp parallel for
    for (int r = 0; r < mat.rows; r++) {
        for (size_t c = 0; c < mat.columns; c++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}

void parallel_flipped_unrolled_transpose(const matrix& mat, matrix& mat_t) {

    #pragma omp parallel for
    for (int c = 0; c < mat.columns; c++) {

        size_t r = 0;
        for (; r + 8 <= mat.rows; r += 8) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 1] = mat._matrix[(r + 1) * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 2] = mat._matrix[(r + 2) * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 3] = mat._matrix[(r + 3) * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 4] = mat._matrix[(r + 4) * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 5] = mat._matrix[(r + 5) * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 6] = mat._matrix[(r + 6) * mat.columns + c];
            mat_t._matrix[c * mat_t.columns + r + 7] = mat._matrix[(r + 7) * mat.columns + c];
        }

        for (; r < mat.rows; r++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}

void parallel_flipped_simd_transpose(const matrix& mat, matrix& mat_t) {

    #pragma omp parallel for
    for (int c = 0; c < mat.columns; c++) {

        size_t r = 0;
        for (; r + 8 <= mat.rows; r += 8) {
            _mm256_store_ps(&mat_t._matrix[c * mat_t.columns + r], {
                mat._matrix[r * mat.columns + c],
                mat._matrix[(r + 1) * mat.columns + c],
                mat._matrix[(r + 2) * mat.columns + c],
                mat._matrix[(r + 3) * mat.columns + c],
                mat._matrix[(r + 4) * mat.columns + c],
                mat._matrix[(r + 5) * mat.columns + c],
                mat._matrix[(r + 6) * mat.columns + c],
                mat._matrix[(r + 7) * mat.columns + c]              
            });
        }

        for (; r < mat.rows; r++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}