//
// Created by riv on 19.05.16.
//

#ifndef RINGBUFFER_SPSC_RING_BUFFER_H
#define RINGBUFFER_SPSC_RING_BUFFER_H


#include <cstdio>
#include <vector>
#include <atomic>

template <typename T>
class spsc_ring_buffer {
public:
    explicit spsc_ring_buffer(size_t size = 1024):capacity(size){
        container = std::vector<T>(size);
        head.store(0);
        tail.store(0);
    }
    bool enqueue(T e){
        size_t tail_now = tail.load(std::memory_order_acquire);
        size_t head_now = head.load(std::memory_order_acquire);
        if ((tail_now +1) % capacity == head_now){
            return false;
        }
        container[tail_now] = e;
        tail.store((tail_now+1) % capacity, std::memory_order_release);
        return true;

    }
    bool dequeue(T& e){

        size_t tail_now = tail.load(std::memory_order_acquire);
        size_t head_now = head.load(std::memory_order_acquire);
        if (tail_now == head_now){
            return false;
        }
        e = container[head_now];
        head.store((head_now - 1) %capacity, std::memory_order_release);
        return true;
    }

private:
    size_t capacity;
    std::vector<T> container;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
};


#endif //RINGBUFFER_SPSC_RING_BUFFER_H
