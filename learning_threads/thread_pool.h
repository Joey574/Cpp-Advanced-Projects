#pragma once

#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <future>
#include <queue>

#include "thread_worker.h"

class thread_pool {




	class thread_worker {
		thread_worker(thread_pool* pool) : thread_pool(pool) {}

		void operator()() {
			std::unique_lock<std::mutex> lock(thread_pool->mutex);

			while (!thread_pool->shutdown && !thread_pool->queue.empty()) {

				thread_pool->condition.wait(lock, [this] {
					return thread_pool->shutdown || !thread_pool->queue.empty();
					});

				if (!thread_pool->queue.empty()) {

					auto func = thread_pool->queue.front();
					thread_pool->queue.pop();

					lock.unlock();
					func();
					lock.lock();
				}
			}
		}

	private:
		thread_pool* thread_pool;
	};




private:
	mutable std::mutex mutex;
	std::condition_variable condition;

	std::vector<std::thread> threads;
	bool shutdown;

	std::queue<std::function<void()>> queue;

	int busy_threads;






	template <typename F, typename...Args>
	auto add_task(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {

		// Create a function with bounded parameters ready to execute
		auto func = std::bind(std::forward<F>(f)), std::forward<Args>(args...);

		// Encapsulate it into a shared ptr in order to be able to copy construct / assign
		auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

		// Wrap the task pointer into a void lambda
		auto wrapper_func = [task_ptr]() { (*task_ptr)(); };

		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.push(weapper_func);

			condition.notify_one();
		}

		return task_ptr->get_future;
	}
};