#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <future>

#define THREAD_NUM 10

void test() {
	std::cout << "Output from thread\n";
}

int main()
{
	std::packaged_task<void()> task(test);

	std::future future = task.get_future();

	std::thread thread(std::move(task));

	future.wait_for(std::chrono::seconds(1));
}