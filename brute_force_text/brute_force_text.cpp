#include <iostream>
#include <string>
#include <Windows.h>

void brute_force(const std::string& text, int delay);

int main()
{
	brute_force("python sucks", 10);
}

void brute_force(const std::string& text, int delay) {

	std::string found = "";
	for (int i = 0; i < text.size(); i++) {
		char current = 0;
		char target = text[i];

		while (current != target) {
			std::cout << found << current << "\n";
			Sleep(delay);
			current++;
		}
		found.push_back(current);
	}
	std::cout << found;
}