//
// Created by riv on 29.04.16.
//

#include "rw_lock.h"

void rw_lock::write_lock(){
    mut.lock();
        while (is_writer || (readers_num > 0))
            unlocked.wait(mut);
        is_writer = true;
    mut.unlock();
}

bool rw_lock::try_write_lock() {
    bool res = false;
    mut.lock();
        if (!(is_writer || (readers_num > 0))){
            res = true;
            is_writer = true;
        }
    mut.unlock();
    return res;
}

void rw_lock::write_unlock() {
    mut.lock();
        is_writer = false;
        unlocked.notify_one();
    mut.unlock();
}

void rw_lock::read_lock() {
    mut.lock();
        while (is_writer){
            unlocked.wait(mut);
        }
        readers_num++;
    mut.unlock();
}

bool rw_lock::try_read_lock() {
    bool res = false;
    mut.lock();
        if (!is_writer){
            res = true;
            readers_num++;
        }
    mut.unlock();
    return res;
}

void rw_lock::read_unlock() {
    mut.lock();
        readers_num--;
        if (readers_num == 0){
            unlocked.notify_one();
        }
    mut.unlock();
}











