#include <iostream>
#include <vector>
#include <random>
#include <Windows.h>

void draw_board(std::vector<std::vector<uint8_t>> game_board);
std::vector <std::vector<uint8_t>> generate_food(std::vector<std::vector<uint8_t>> game_board);
std::vector<std::vector<uint8_t>> move_snake(std::vector<std::vector < uint8_t>> game_board, uint8_t direction);

/*
Game State
0 -> empty space
1 -> snake body
2 -> food
3 -> snake head
*/

enum direction {
	up, down, left, right
};

int main()
{
	int width = 20;
	int height = 20;

	std::vector<std::vector<uint8_t>> game_board;

	for (int i = 0; i < height; i++) {
		game_board.emplace_back(std::vector<uint8_t>(width, 0));
	}

	game_board[13][10] = 3;

	move_snake(game_board, 1);
}

std::vector<std::vector<uint8_t>> generate_food(std::vector<std::vector<uint8_t>> game_board) {
	
	int min = 0;
	int max = game_board.size() - 1;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(min, max);

	while (true) {
		int rand_x = distrib(gen);
		int rand_y = distrib(gen);

		if (game_board[rand_x][rand_y] == 0) {
			game_board[rand_x][rand_y] = 2;
			break;
		}
	}

	return game_board;
}

std::vector<std::vector<uint8_t>> move_snake(std::vector<std::vector < uint8_t>> game_board, direction dir) {

	int y = 0;
	int x = 0;
	int index = 0;

	// Find position of snake head
	while (game_board[x][y] != 3) {
		index++;

		x = index % game_board.size();
		y = index / game_board.size();
	}
	std::cout << x << " :: " << y;

	if (dir == direction::up) {

	} else if (dir == direction::down) {

	} else if (dir == direction::left) {

	} else if (dir == direction::right) {

	}

	return game_board;
}

void draw_board(std::vector<std::vector<uint8_t>> game_board) {

	for (int i = 0; i < game_board.size(); i++) {
		for (int x = 0; x < game_board[i].size(); x++) {
			if (game_board[i][x] == 0) {
				std::cout << ". ";
			} else if (game_board[i][x] == 1) {
				std::cout << "O ";
			} else if (game_board[i][x] == 2) {
				std::cout << "X ";
			} else if (game_board[i][x] == 3) {
				std::cout << "@ ";
			}
		}
		std::cout << "\n";
	}
}