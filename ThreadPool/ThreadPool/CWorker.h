#pragma once
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>


class CWorker{
public:

	CWorker(){
		enabled = true;
		thread(&CWorker::thread_fn, this);
	}
	~CWorker(){
		enabled = false;
		cond_var.notify_one();
		thread.join();
	}
	void add_job(std::function<void()> fn){
		std::unique_lock<std::mutex> locker(mutex);
		func_queue.push(fn);
		cond_var.notify_one();
	}
	size_t getTaskCount()
	{
		std::unique_lock<std::mutex> locker(mutex);
		return func_queue.size();
	}
	bool  isEmpty()
	{
		std::unique_lock<std::mutex> locker(mutex);
		return func_queue.empty();
	}

private:

	bool enabled;
	std::condition_variable	cond_var;
	std::queue<std::function<void()>> func_queue;
	std::mutex mutex;
	std::thread	thread;

	void thread_fn() {
		while (enabled)
			{
				std::unique_lock<std::mutex> locker(mutex);
				cond_var.wait(locker, [&]() { return !func_queue.empty() || !enabled; });
				while (!func_queue.empty()) {
					std::function<void()> fn = func_queue.front();
					locker.unlock();
					fn();
					locker.lock();
					func_queue.pop();
				}
			}
	}
};