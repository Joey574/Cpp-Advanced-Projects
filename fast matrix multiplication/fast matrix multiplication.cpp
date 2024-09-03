#include <iostream>
#include <vector>
#include <string>
#include <random>

struct matrix {
	int rows;
	int columns;
	std::vector<float> matrix;
};

std::string matrix_to_string(matrix a);
void random_init(matrix& a);
matrix old_dot_product(matrix a, matrix b);

int main()
{
	matrix a;
	matrix b;

	a.rows = 10; a.columns = 10;
	b.rows = 10; b.columns = 10;

	a.matrix = std::vector<float>(a.rows * a.columns);
	b.matrix = std::vector<float>(b.rows * b.columns);

	for (int r = 0; r < a.rows; r++) {
		for (int c = 0; c < a.columns; c++) {
			a.matrix[r * a.columns + c] = r;
		}
	}

	for (int r = 0; r < b.rows; r++) {
		for (int c = 0; c < b.columns; c++) {
			b.matrix[r * b.columns + c] = r;
		}
	}

	std::cout << "a:\n" << matrix_to_string(a);
	std::cout << "\nb:\n" << matrix_to_string(b);

	matrix c = old_dot_product(a, b);

	std::cout << "\n\nc:\n" << matrix_to_string(c);
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

	for (int i = 0; i < a.rows * a.columns; i++) {
		a.matrix[i] = dist(gen);
	}
}

matrix old_dot_product(matrix a, matrix b) {
	matrix c; c.rows = a.rows; c.columns = b.columns;
	c.matrix = std::vector<float>(c.rows * c.columns);

	for (int r = 0; r < a.rows; r++) {
		for (int k = 0; k < b.rows; k++) {
			for (int j = 0; j < b.columns; j++) {
				c.matrix[r * b.columns + j] += a.matrix[r * a.columns + k] * b.matrix[k * b.columns + j];
			}
		}
	}

	return c;
}