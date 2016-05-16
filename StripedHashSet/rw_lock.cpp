//
// Created by riv on 29.04.16.
//

#include "rw_lock.h"

void rw_lock::write_lock(){
    std::unique_lock<std::mutex> uni_lock(*mut);
    uni_lock.lock();
        while (is_writer || (readers_num > 0))
            unlocked->wait(uni_lock);
        is_writer = true;
}

bool rw_lock::try_write_lock() {
    std::unique_lock<std::mutex> uni_lock(*mut);
    bool res = false;
    uni_lock.lock();
        if (!(is_writer || (readers_num > 0))){
            res = true;
            is_writer = true;
        }
    return res;
}

void rw_lock::write_unlock() {
    std::unique_lock<std::mutex> uni_lock(*mut);
    uni_lock.lock();
        is_writer = false;
        unlocked->notify_one();
}

void rw_lock::read_lock() {
    std::unique_lock<std::mutex> uni_lock(*mut);
    uni_lock.lock();
        while (is_writer){
            unlocked->wait(uni_lock);
        }
        readers_num++;
}

bool rw_lock::try_read_lock() {
    std::unique_lock<std::mutex> uni_lock(*mut);
    bool res = false;
    uni_lock.lock();
        if (!is_writer){
            res = true;
            readers_num++;
        }
    return res;
}

void rw_lock::read_unlock() {
    std::unique_lock<std::mutex> uni_lock(*mut);
    uni_lock.lock();
        readers_num--;
        if (readers_num == 0){
            unlocked->notify_one();
        }
}

rw_lock::rw_lock() {
    mut  = new std::mutex();
    unlocked = new std::condition_variable();
}

rw_lock::rw_lock(rw_lock&& some){
    readers_num = some.readers_num;
    unlocked = some.unlocked;
    mut = some.mut;
    is_writer = some.is_writer;

}













