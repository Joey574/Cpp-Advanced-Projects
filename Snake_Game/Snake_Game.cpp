#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <conio.h>
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

	pos p; p.x = width / 2; p.y = height / 2;
	snake.push_back(p);
	game_board[p.y][p.x] = tile_type::head;
	p.x--;
	snake.push_back(p);
	game_board[p.y][p.x] = tile_type::body;
	p.x--;
	snake.push_back(p);
	game_board[p.y][p.x] = tile_type::body;

	const int targetFPS = 5;
	const std::chrono::milliseconds frameDuration(1000 / targetFPS);
	direction dir = direction::right;

	game_board = generate_food(game_board);

	while (true) {
		auto start = std::chrono::high_resolution_clock::now();

		// Game loop here

		// Check for key press and change direction
		if (_kbhit()) {
			char key = _getch();

			if (key == 'w') {
				dir = direction::up;
			} else if (key == 'a') {
				dir = direction::left;
			} else if (key == 's') {
				dir = direction::down;
			} else if (key == 'd') {
				dir = direction::right;
			}
		}

		game_board = move_snake(game_board, dir);

		std::cout << "\u001b[H";
		draw_board(game_board);

		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		auto sleepTime = frameDuration - elapsed;

		if (sleepTime > std::chrono::milliseconds(0)) {
			std::this_thread::sleep_for(sleepTime);
		}
	}
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

	// remove tail
	game_board[snake[snake.size() - 1].y][snake[snake.size() - 1].x] = tile_type::none;

	// update snake position
	for (int i = snake.size() - 1; i > 0; i--) {
		snake[i] = snake[i - 1];
	}

	// set head -> body, update head location
	game_board[snake[0].y][snake[0].x] = tile_type::body;
	if (dir == direction::up) {
		snake[0].y--;
	}
	else if (dir == direction::down) {
		snake[0].y++;
	}
	else if (dir == direction::left) {
		snake[0].x--;
	}
	else if (dir == direction::right) {
		snake[0].x++;
	}

	if (game_board[snake[0].y][snake[0].x] == tile_type::food) {
		pos p = snake[snake.size() - 1];
		snake.push_back(p);
		game_board = generate_food(game_board);
	}

	game_board[snake[0].y][snake[0].x] = tile_type::head;

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