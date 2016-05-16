#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>



template <typename T>
class CBlockingQueue {
private:
	std::queue<T> internalQueue;
	std::condition_variable condVar;
	std::mutex mut;

public:
	CBlockingQueue(){}
	~CBlockingQueue(){}
	void push(const T& obj) {
		std::unique_lock<std::mutex> lock(mut);
			internalQueue.push(obj);
			lock.unlock();
		condVar.notify_one();
	}
	T wait_pop() {
		std::unique_lock<std::mutex> guard(mut); //mew
		if (!internalQueue.empty()) {
			T result(this->internalQueue.front());
			internalQueue.pop();
			return result;
		}
		else {
			while (1) {
				this->condVar.wait(guard, [this]() {return this->internalQueue.size() > 0; });
				T result(this->internalQueue.front());
				internalQueue.pop();
				return result;
			}
		}
	}
	
	std::pair<bool, T> try_pop() {
		std::lock_guard<std::mutex> guard(mut);
			if (!internalQueue.empty()) {
				T result(internalQueue.front());
				internalQueue.pop();
				return std::make_pair(true, result);
			}
			else {
				return std::make_pair(false, T());
			}
	}
};