#pragma once 
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <atomic>

template <int N>
class CCyclicBarrier {
public:
	CCyclicBarrier() {
		howManyWaiting = 0;
		iteration = 0;
		isNotWaiting = false;
	}
	void enter() {
		if (howManyWaiting.fetch_add(1) >= N - 1) {
			isNotWaiting = true;
			condVar.notify_all();
			howManyWaiting.store(0);
			iteration.fetch_add(1);
		}
		else {
			unsigned int generation = iteration.load();
			std::unique_lock<std::mutex> lock(mutex);
			condVar.wait(lock, [&] { return isNoWaiting; });
			if (generation == iteration.load()) {
				condVar.wait(lock, [&] { return isNoWaiting; });
			}
		}
	}
	~CCyclicBarrier() {}
private:
	std::atomic<unsigned int> howManyWaiting;
	std::mutex mut;
	std::condition_variable condVar;
	std::atomic<unsigned int> iteration;
	std::atomic<bool> isNotWaiting;
};