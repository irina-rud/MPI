#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

std::atomic<bool> isWaiting;
std::condition_variable condVar;
std::mutex mutex;

void stepRight() {
	for (int i; i < 10; ++i) {
		std::unique_lock<std::mutex> lock(mutex);
		std::cout << "right" << std::endl;
		if (isWaiting.exchange != true) {
			condVar.notify_one();
		}
		else {
			condVar.wait(lock);
		}
	}
}
void stepLeft() {
	for (int i; i < 10; ++i) {
		std::unique_lock<std::mutex> lock(mutex);
		std::cout << "left" << std::endl;
		if (isWaiting.exchange != true) {
			condVar.notify_one();
		}
		else {
			condVar.wait(lock);
		}
	}
}

int main() {
	std::thread tr1(stepLeft);
	std::thread tr2(stepRight);
	
	tr1.join();
	tr2.join();
}
