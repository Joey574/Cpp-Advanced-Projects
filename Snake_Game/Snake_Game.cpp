#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <conio.h>

#include "Matrix.h"

struct pos {
	int x;
	int y;
};
enum class tile_type {
	none, food, body, head
};
enum class direction {
	up, down, left, right
};

void draw_board(std::vector<std::vector<tile_type>> game_board);
void manual_food_snake_input(direction& snake_dir, direction& food_dir);
std::vector <std::vector<tile_type>> generate_snake(std::vector<std::vector<tile_type>> game_board);
std::vector <std::vector<tile_type>> generate_food(std::vector<std::vector<tile_type>> game_board);
std::vector <std::vector<tile_type>> update_snake(std::vector<std::vector<tile_type>> game_board, direction snake_dir);
std::vector <std::vector<tile_type>> update_food(std::vector<std::vector<tile_type>> game_board, direction food_dir);
std::vector <std::vector<tile_type>> check_collisions(std::vector<std::vector<tile_type>> game_board);


std::vector<pos> snake;
pos food;

int main()
{
	int width = 20;
	int height = 20;

	std::vector<std::vector<tile_type>> game_board;

	for (int i = 0; i < height; i++) {
		game_board.emplace_back(std::vector<tile_type>(width, tile_type::none));
	}

	game_board = generate_snake(game_board);

	const int targetFPS = 5;
	const std::chrono::milliseconds frameDuration(1000 / targetFPS);

	// Initialize food and start direction
	direction snake_dir = direction::right;
	direction food_dir = direction::right;
	game_board = generate_food(game_board);

	auto begin_clock = std::chrono::high_resolution_clock::now();

	int food_step = 1;

	while (true) {
		auto start = std::chrono::high_resolution_clock::now();

		manual_food_snake_input(snake_dir, food_dir);

		game_board = update_snake(game_board, snake_dir);

		if (food_step == 2) {
			food_step = 0;
			game_board = update_food(game_board, food_dir);
		}

		game_board = check_collisions(game_board);

		std::cout << "\u001b[H";
		draw_board(game_board);

		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		auto sleepTime = frameDuration - elapsed;

		if (sleepTime > std::chrono::milliseconds(0)) {
			std::this_thread::sleep_for(sleepTime);
		}
		food_step++;
	}
}


void manual_food_snake_input(direction &snake_dir, direction &food_dir) {
	if (_kbhit()) {
		char key = _getch();

		if (key == 'w') {
			snake_dir = direction::up;
		}
		else if (key == 'a') {
			snake_dir = direction::left;
		}
		else if (key == 's') {
			snake_dir = direction::down;
		}
		else if (key == 'd') {
			snake_dir = direction::right;
		}

		if (key == 'i') {
			food_dir = direction::up;
		}
		else if (key == 'j') {
			food_dir = direction::left;
		}
		else if (key == 'k') {
			food_dir = direction::down;
		}
		else if (key == 'l') {
			food_dir = direction::right;
		}
	}
}


std::vector<std::vector<tile_type>> generate_snake(std::vector<std::vector<tile_type>> game_board) {
	pos p; p.x = game_board[0].size() / 2; p.y = game_board.size() / 2;
	snake.push_back(p);
	game_board[p.y][p.x] = tile_type::head;
	p.x--;
	snake.push_back(p);
	game_board[p.y][p.x] = tile_type::body;
	p.x--;
	snake.push_back(p);
	game_board[p.y][p.x] = tile_type::body;
	return game_board;
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

		if (game_board[rand_y][rand_x] == tile_type::none) {
			game_board[rand_y][rand_x] = tile_type::food;
			food.x = rand_x; food.y = rand_y;
			break;
		}
	}

	return game_board;
}


std::vector<std::vector<tile_type>> update_snake(std::vector<std::vector<tile_type>> game_board, direction snake_dir) {
	// remove tail and set head
	game_board[snake[snake.size() - 1].y][snake[snake.size() - 1].x] = tile_type::none;
	game_board[snake[0].y][snake[0].x] = tile_type::body;

	// update snake positions
	for (int i = snake.size() - 1; i > 0; i--) {
		snake[i] = snake[i - 1];
	}

	// update head location
	if (snake_dir == direction::up) {
		snake[0].y--;
	}
	else if (snake_dir == direction::down) {
		snake[0].y++;
	}
	else if (snake_dir == direction::left) {
		snake[0].x--;
	}
	else if (snake_dir == direction::right) {
		snake[0].x++;
	}
	return game_board;
}

std::vector<std::vector<tile_type>> update_food(std::vector<std::vector<tile_type>> game_board, direction food_dir) {
	// update food position
	game_board[food.y][food.x] = tile_type::none;
	if (food_dir == direction::up) {
		food.y--;
	}
	else if (food_dir == direction::down) {
		food.y++;
	}
	else if (food_dir == direction::left) {
		food.x--;
	}
	else if (food_dir == direction::right) {
		food.x++;
	}

	return game_board;
}

std::vector<std::vector<tile_type>> check_collisions(std::vector<std::vector<tile_type>> game_board) {

	// check for snake out of bounds
	if (snake[0].y < 0 || snake[0].x < 0 || snake[0].y >= game_board.size() || snake[0].x >= game_board[0].size()) {

		for (int i = 1; i < snake.size(); i++) {
			game_board[snake[i].y][snake[i].x] = tile_type::none;
		}

		snake.clear();
		game_board = generate_snake(game_board);
	}

	// check for food out of bounds
	if (food.x < 0 || food.y < 0 || food.x >= game_board[0].size() || food.y >= game_board.size()) {
		game_board = generate_food(game_board);

		int x = std::max(0, std::min((int)(game_board[0].size()), food.x));
		int y = std::max(0, std::min((int)(game_board.size()), food.y));

		game_board[y][x] == tile_type::none;
	}

	// check for collision within snake
	if (game_board[snake[0].y][snake[0].x] == tile_type::body) {
		for (int i = 1; i < snake.size(); i++) {
			game_board[snake[i].y][snake[i].x] = tile_type::none;
		}

		snake.clear();
		game_board = generate_snake(game_board);
	}

	// check for collisions with food
	for (int i = 0; i < snake.size(); i++) {
		if (snake[i].x == food.x && snake[i].y == food.y) {
			snake.push_back(snake[snake.size() - 1]);
			game_board = generate_food(game_board);
		}
	}

	game_board[food.y][food.x] = tile_type::food;
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