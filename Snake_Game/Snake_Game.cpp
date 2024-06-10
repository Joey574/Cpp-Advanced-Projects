#include <iostream>
#include <vector>
#include <random>
#include <Windows.h>

struct pos {
	int x;
	int y;
};
enum tile_type {
	none, food, body, head
};
enum direction {
	up, down, left, right
};

void draw_board(std::vector<std::vector<tile_type>> game_board);
std::vector <std::vector<tile_type>> generate_food(std::vector<std::vector<tile_type>> game_board);
std::vector<std::vector<tile_type>> move_snake(std::vector<std::vector < tile_type>> game_board, direction dir);

std::vector<pos> snake;

int main()
{
	int width = 20;
	int height = 20;

	std::vector<std::vector<tile_type>> game_board;

	for (int i = 0; i < height; i++) {
		game_board.emplace_back(std::vector<tile_type>(width, tile_type::none));
	}

	snake.emplace_back(height / 2, width / 2);
	snake.emplace_back(height / 2, width / 2 - 1);
	snake.emplace_back(height / 2, width / 2 - 2);

	draw_board(game_board);
	game_board = move_snake(game_board, direction::up);

	std::cout << "\n\n";

	draw_board(game_board);
}

std::vector<std::vector<tile_type>> generate_food(std::vector<std::vector<tile_type>> game_board) {
	
	int min = 0;
	int max = game_board.size() - 1;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(min, max);

	while (true) {
		int rand_x = distrib(gen);
		int rand_y = distrib(gen);

		if (game_board[rand_x][rand_y] == 0) {
			game_board[rand_x][rand_y] = tile_type::food;
			break;
		}
	}

	return game_board;
}

std::vector<std::vector<tile_type>> move_snake(std::vector<std::vector<tile_type>> game_board, direction dir) {

	game_board[snake[0].y][snake[0].x] = tile_type::body;

	if (dir == direction::up) {
		game_board[snake[0].y - 1][snake[0].x] = tile_type::head;
		snake[0].y--;
	} else if (dir == direction::down) {
		game_board[snake[0].y + 1][snake[0].x] = tile_type::head;
		snake[0].y++;
	} else if (dir == direction::left) {
		game_board[snake[0].y][snake[0].x - 1] = tile_type::head;
		snake[0].x--;
	} else if (dir == direction::right) {
		game_board[snake[0].y][snake[0].x + 1] = tile_type::head;
		snake[0].x++;
	}

	return game_board;
}

void draw_board(std::vector<std::vector<tile_type>> game_board) {

	for (int i = 0; i < game_board.size(); i++) {
		for (int x = 0; x < game_board[i].size(); x++) {
			if (game_board[i][x] == tile_type::none) {
				std::cout << ". ";
			} else if (game_board[i][x] == tile_type::body) {
				std::cout << "O ";
			} else if (game_board[i][x] == tile_type::food) {
				std::cout << "X ";
			} else if (game_board[i][x] == tile_type::head) {
				std::cout << "@ ";
			}
		}
		std::cout << "\n";
	}
}