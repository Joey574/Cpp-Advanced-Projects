#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <Windows.h>
#include <random>

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

void parallel_omp_simd_transpose(const matrix& mat, matrix& mat_t);



void random_init(matrix& a) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

    a._matrix = std::vector<float>(a.rows * a.columns);
    for (int i = 0; i < a.rows * a.columns; i++) {
        a._matrix[i] = dist(gen);
    }
}

int main()
{


    run_test("flipped_transpose", flipped_transpose);
    run_test("basic_transpose", basic_transpose);

    run_test("parallel_flipped_transpose", parallel_flipped_transpose);
    run_test("parallel_transpose", parallel_transpose);

    run_test("parallel_omp_simd_transpose", parallel_omp_simd_transpose);
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
        }

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
    for (int c = 0; c < mat.columns; c++) {
        for (int r = 0; r < mat.rows; r++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}
void basic_transpose(const matrix& mat, matrix& mat_t) {
    for (int r = 0; r < mat.rows; r++) {
        for (int c = 0; c < mat.columns; c++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}

void parallel_flipped_transpose(const matrix& mat, matrix& mat_t) {

    #pragma omp parallel for
    for (int c = 0; c < mat.columns; c++) {
        for (int r = 0; r < mat.rows; r++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}
void parallel_transpose(const matrix& mat, matrix& mat_t) {

    #pragma omp parallel for
    for (int r = 0; r < mat.rows; r++) {
        for (int c = 0; c < mat.columns; c++) {
            mat_t._matrix[c * mat_t.columns + r] = mat._matrix[r * mat.columns + c];
        }
    }
}