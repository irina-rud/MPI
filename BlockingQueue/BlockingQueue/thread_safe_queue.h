#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>


template <typename T, class Container = std::deque<T>>
class thread_safe_queue {
private:
	Container internalQueue;
	std::condition_variable condVar;
	std::mutex mut;
	size_t capacity;
	std::atomic<bool> flag;

public:
	explicit thread_safe_queue():flag(false), capacity(static_cast<size_t> (-1)) {}

	explicit thread_safe_queue(size_t cap): capacity(cap){}

	~thread_safe_queue(){}

	void enqueue(const T& obj) {
		std::unique_lock<std::mutex> lock(mut);
			if (flag.load()){
				lock.unlock();
				throw new std::exception();
			}
			if (internalQueue.size() < capacity && flag == (false)  && capacity != (static_cast<size_t> (-1))){
				internalQueue.push_back(obj);
			}
			lock.unlock();
		condVar.notify_one();
	}

	void pop(T& val) {
		std::unique_lock<std::mutex> guard(mut); //mew
		if (!internalQueue.empty()) {
			T result(this->internalQueue.front());
			internalQueue.pop_front();
			val = std::move(result);
		}
		else {
			if (flag.load()){
				guard.unlock();
				throw new std::exception();
			}
			while (1) {
				this->condVar.wait(guard, [this]() {return this->internalQueue.size() > 0; });
				T result(this->internalQueue.front());
				internalQueue.pop_front();
				val = std::move(result);
				return;
			}
		}
	}
	void shutdown(){
		flag.store(true);
	}

	std::pair<bool, T> try_pop() {
		std::lock_guard<std::mutex> guard(mut);
			if (!internalQueue.empty()) {
				T result(internalQueue.front());
				internalQueue.pop_front();
				return std::make_pair(true, result);
			}
			else {
				return std::make_pair(false, T());
			}
	}
};