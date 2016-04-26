#include <mutex>
#include <condition_variable>

//we should include atomic here

class blocking_flag {
public:
    blocking_flag()
        : ready_(false)
    {}

    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        while (!ready_.load()) {
            ready_cond_.wait(lock);
        }
    }

    void set() {
        ready_.store(true);
        ready_cond_.notify_all();
    }

private:
    std::atomic<bool> ready_;
    std::mutex mtx_;
    std::condition_variable ready_cond_;
};

//This usage is correct, because we use flag only one time, but if we use it twice or more,
//we wouldn't have always true.
//all operations are atomic, so it's good
//if nobody call set(), we will warm the air.

#include <thread>
#include <iostream>

int main() {
    blocking_flag f;

    std::thread t(
        [&f]() {
            f.wait();
            std::cout << "ready!" << std::endl;
        }
    );

    f.set();
    t.join();

    return 0;
}