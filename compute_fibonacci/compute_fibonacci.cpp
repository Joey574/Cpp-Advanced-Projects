#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_map>

void test(int(*operation)(int), std::string name);
int recursive_compute(int n);
int memoisation_compute(int n);
int linear_compute(int n);

std::unordered_map<int, int> _memoisation_map;

int main()
{
	_memoisation_map[0] = 1;
	_memoisation_map[1] = 1;


	test(recursive_compute, "Recursive");
	//test(memoisation_compute, "Recursive_memoisation");
	//test(linear_compute, "Linear");
}

void test(int(*operation)(int), std::string name) {
	int order = 0;
	std::chrono::duration<double, std::milli> time;

	auto start = std::chrono::high_resolution_clock::now();
	while (time.count() < 1000) {
		int r = (*operation)(order);
		order++;
		time = std::chrono::high_resolution_clock::now() - start;
	}
	std::cout << name << ": " << order << " orders\n";
}

int recursive_compute(int n) {
	if (n == 0 || n == 1) {
		return 1;
	}
	return recursive_compute(n - 1) + recursive_compute(n - 2);
}

int memoisation_compute(int n) {
	if (_memoisation_map.find(n) != _memoisation_map.end()) {
		return _memoisation_map[n];
	}

	_memoisation_map[n] = memoisation_compute(n - 1) + memoisation_compute(n - 2);
	return _memoisation_map[n];
}

int linear_compute(int n) {

	int n_a = 1;
	int n_1 = 1;
	int n_2 = 1;

	for (int i = 2; i < n; i++) {
		n_a = n_1 + n_2;
		n_2 = n_1;
		n_1 = n_a;
	}

	return n_a;
}