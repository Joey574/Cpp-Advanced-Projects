#include <iostream>
#include <thread>
#include <string>
#include <vector>

#define THREAD_NUM 10

void test() {
	std::cout << "Output from thread\n";
}

int main()
{
	std::vector<float> a(100, 5);
	std::vector<float> b(100, 2);

	std::thread threads[THREAD_NUM];

	for (std::thread& thread : threads) {
		thread.join();
	}
}