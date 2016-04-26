#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Semaphore.h"


static std::atomic_int stepCountParity = ATOMIC_VAR_INIT(0);
static std::condition_variable condVarRight;
static std::condition_variable condVarLeft;
static std::mutex mutex;

void stepLeft() {
	std::unique_lock<std::mutex> lock(mutex);
	for (int i = 0; i < 10; ++i) {
        while (stepCountParity.load() == 0)
            condVarRight.wait(lock);
        stepCountParity.store(0);
        std::cout << "left" << std::endl;
        condVarLeft.notify_one();
	}
}
void stepRight() {
    std::unique_lock<std::mutex> lock(mutex);
    for (int i = 0; i < 10; ++i) {
        while (stepCountParity.load() == 1)
            condVarLeft.wait(lock);
        stepCountParity.store(1);
        std::cout << "right" << std::endl;
        condVarRight.notify_one();
    }
}

void condVarWalk() {
	std::thread tr1(stepLeft);
	std::thread tr2(stepRight);

	if (tr1.joinable()) {
		tr1.join();
	}
	if (tr2.joinable()) {
		tr2.join();
	}
}

//дальше семафоры

static Semaphore semLeft(1);
static Semaphore semRight(0);

void walkLeft2() {
	int i = 0;
    for (int i = 0;i++ <= 10; ++i) {
		semRight.wait();
		std::cout << "step left" << std::endl;
		semLeft.signal();
	}
}
void walkRight2() {
	for (int i = 0;i++ <= 10; ++i) {
		semLeft.wait();
		std::cout << "step right" << std::endl;
		semRight.signal();
	}
}
void semaphoreWalking() {
	std::thread thread1(walkLeft2);
	std::thread thread2(walkRight2);

	if (thread1.joinable()) {
		thread1.join();
	}
	if (thread2.joinable()) {
		thread2.join();
	}
}




int main() {
    std::cout << "Robot initialised :)" << std::endl;
    semaphoreWalking();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    condVarWalk();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 0;
}


