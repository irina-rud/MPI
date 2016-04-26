
#include "stdafx.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/types.h>
#include <sys/sem.h>

std::atomic_int isWaiting = ATOMIC_VAR_INIT(0);
std::condition_variable condVar;
std::mutex mutex;

void stepRight() {
	std::unique_lock<std::mutex> lock(mutex);
	for (int i = 0; i < 10; ++i) {
		std::cout << "right" << std::endl;
		isWaiting.fetch_add(1);
		if (isWaiting.load() % 2 != 1) {
			condVar.notify_one();
		}
		else {
			condVar.wait(lock);
		}
	}
}
void stepLeft() {
	std::unique_lock<std::mutex> lock(mutex);
	for (int i = 0; i < 10; ++i) {
		std::cout << "left" << std::endl;
		isWaiting.fetch_add(1);
		if (isWaiting.load() % 2 != 0) {
			condVar.notify_one();
		}
		else {
			condVar.wait(lock);
		}
	}
}

void walkLeft() {


}

int main() {
	std::thread tr1(stepLeft);
	std::thread tr2(stepRight);
	
	tr1.join();
	tr2.join();


}


