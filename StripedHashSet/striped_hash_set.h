//
// Created by riv on 29.04.16.
//

#ifndef STRIPEDHASHSET_STRIPEDHASHSET_H
#define STRIPEDHASHSET_STRIPEDHASHSET_H

#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include "rw_lock.h"

template<typename T, class H = std::hash<T>>
class striped_hash_set {
public:
    striped_hash_set(int num_stripes = 5 , int growth_factor = 2, int max_load_factor = 5);
    void add(const T& e);
    void remove(const T& e);
    bool contains(const T& e)const;

private:
    int size;
    std::mutex size_lock;
    int capasity;
    int growth_factor;
    int max_load_factor;
    std::vector<rw_lock> stripe_locks;
    std::vector<std::list<T>>   buckets;
    void condense();
    void expand();
    bool lock_all_mutexes();
    void rehash(unsigned long new_buckets_num);
    void unlock_all_mutexes();
};


template<typename T, class H>
striped_hash_set<T,H>::striped_hash_set(int stripes , int growth_fact, int max_load_fact){
    growth_factor = growth_fact;
    max_load_factor = max_load_fact;
    size = 0;
    stripe_locks.resize((unsigned long) stripes);
    buckets.resize((unsigned long) stripes);
}

template<typename T, class H>
bool striped_hash_set<T,H>::contains(const T &element) const{
    unsigned int witch_back = H(element) % this->buckets.size();
    unsigned int witch_mutex = (unsigned int) (witch_back % this->stripe_locks.size());
    this->stripe_locks[witch_mutex].read_lock();
    for (const auto& it : this->buckets[witch_back]) {
        if (it == element) {
            this->stripe_locks[witch_mutex].read_unlock();
            return true;
        }
    }
    this->stripe_locks[witch_mutex].read_unlock();
    return false;
}

template<typename T, class H>
void striped_hash_set<T,H>::remove(const T &element) {
    this->condense();
    unsigned int witch_bucket = H(element) % this->buckets.size();
    unsigned long witch_mutex = witch_bucket % this->stripe_locks.size();
    stripe_locks[witch_mutex].write_lock();
    for (auto it = buckets[witch_bucket].before_begin();
         it._M_next() != buckets[witch_bucket].end(); it++)
    {
        if (*(it._M_next()) == element) {
            buckets[witch_bucket].erase_after(it);
            std::unique_lock<std::mutex> lock(size_lock);
            size--;
            stripe_locks[witch_mutex].write_unlock();
            return;
        }
    }
    stripe_locks[witch_mutex].write_unlock();
}

template<typename T, class H>
void striped_hash_set<T,H>::add(const T &element) {
    if (this->contains(element)) {
        return;
    }
    if ((double)this->size / (double)this->capasity >= max_load_factor  ){
        expand();
    }
    unsigned int witch_back = H(element) % this->buckets.size();
    unsigned int witch_mutex = (unsigned int) (witch_back % this->stripe_locks.size());
    stripe_locks[witch_mutex].write_lock();
    buckets[witch_back].push_front(element);
    stripe_locks[witch_mutex].write_lock();
    std::unique_lock<std::mutex> uniq_size_lock(size_lock);
    this->size++;
}


template<typename T, class H>
void striped_hash_set<T,H>::condense() {
    unsigned long old_buckets_num = this->buckets.size();
    unsigned long new_buckets_num = (old_buckets_num / growth_factor);
    if (new_buckets_num < this->stripe_locks.size()) {
        return;
    }
    int new_load_factor = (int) std::round(size / new_buckets_num);
    if (new_load_factor > max_load_factor) {
        return;
    }
    bool flag = lock_all_mutexes();
    if (flag) {
        rehash(new_buckets_num);
        buckets.resize(new_buckets_num);
        unlock_all_mutexes();
    }
}


template<class T, class H>
bool striped_hash_set<T, H>::lock_all_mutexes(){
    unsigned long old_buckets_number = this->buckets.size();
    stripe_locks[0].write_lock();
    if (old_buckets_number!= this->buckets.size()) {
        stripe_locks[0].write_unlock();
        return false;
    }
    for (int i = 1; i < stripe_locks.size(); ++i)
        stripe_locks[i].write_lock();
    return true;
}


template<class T, class H>
void striped_hash_set<T, H>::unlock_all_mutexes() {
    for (auto it : stripe_locks) {
        it.write_unlock();
    }
}

template<class T, class H>
void striped_hash_set<T, H>::expand() {
    if ( double(size) / buckets.size() < max_load_factor) {
        return;
    }
    bool is_locked = lock_all_mutexes();
    if (is_locked) {
        unsigned long new_buckets_num = growth_factor * this->buckets.size();
        buckets.resize(new_buckets_num);
        rehash(new_buckets_num);
        unlock_all_mutexes();
    }
}



template<class T, class H>
void striped_hash_set<T, H>::rehash(unsigned long i) {
    for(int i = 0; i < buckets.size(); ++i) {
        for(auto it = buckets[i].before_begin(); it._M_next() != buckets[i].end();) {
            unsigned int new_hash = H(*(it._M_next()));
            unsigned long new_bucket = new_hash % buckets.size();
            if (new_bucket != i) {
                buckets[new_bucket].push_front(*(it._M_next()));
                buckets[i].erase_after(it);
            } else {
                it++;
            }
        };
    };

}

#endif //STRIPEDHASHSET_STRIPEDHASHSET_H

// thread safe hash map