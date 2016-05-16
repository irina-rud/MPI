//
// Created by riv on 29.04.16.
//

#ifndef STRIPEDHASHSET_RWLOCK_H
#define STRIPEDHASHSET_RWLOCK_H

#include <iostream>
#include <mutex>
#include <condition_variable>

class rw_lock {
public:
    rw_lock();

    rw_lock(rw_lock && some);

    void write_lock();
    bool try_write_lock();
    void write_unlock();

    void read_lock();
    bool try_read_lock();
    void read_unlock();

private:
    size_t readers_num;
    std::mutex* mut;
    bool is_writer;
    std::condition_variable* unlocked;
};


#endif //STRIPEDHASHSET_RWLOCK_H
