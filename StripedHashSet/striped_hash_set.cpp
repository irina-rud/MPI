//
// Created by riv on 29.04.16.
//

#include <list>
#include <cmath>
#include "striped_hash_set.h"

template<typename T, class H = std::hash<T>>
striped_hash_set<T,H>::striped_hash_set(int num_stripes = 5 , int growth_factor = 2, int max_load_factor = 5){
    this->num_stripes = num_stripes;
    this->growth_factor = growth_factor;
    this->max_load_factor = max_load_factor;
    this->size = 0;
    this->stripe_locks.resize((unsigned long) num_stripes);
    this->buckets.resize((unsigned long) num_stripes);
}

template<typename T, class H = std::hash<T>>
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

template<typename T, class H = std::hash<T>>
void striped_hash_set::remove(const T &element) {
    this->condense();
    unsigned int witch_bucket = H(element) % this->buckets.size();
    unsigned long witch_mutex = witch_bucket % this->stripe_locks.size();
    stripe_locks[witch_mutex].write_lock();
    for (auto it = buckets[witch_bucket].before_begin();
         it._M_next() != buckets[witch_bucket].end(); it++)
    {
        if (*(it._M_next()) == element) {
            // Если следующий за нами элемент подлежит удалению, то удалим его.
            buckets[witch_bucket].erase_after(it);
            std::unique_lock<std::mutex> lock(size_lock);
                size--;
            stripe_locks[witch_mutex].write_unlock();
            return;
        }
    }
    stripe_locks[witch_mutex].write_unlock();
}

template<typename T, class H = std::hash<T>>
void striped_hash_set::add(const T &element) {
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


template<typename T, class H = std::hash<T>>
void striped_hash_set::condense() {
    // Посчитаем новое количество элементов и новый load factor.
    unsigned long old_buckets_num = this->buckets.size();
    unsigned long new_buckets_num = (old_buckets_num / growth_factor);
    // Число корзин должно быть всегда не меньше числа мьютексов(страйпов).
    if (new_buckets_num < this->stripe_locks.size()) {
        return;
    }
    // Посчитаем новый, будущий load factor.
    int new_load_factor = (int) std::round(size / new_buckets_num);
    // Проверим, нужно ли нам сокращать таблицу в соответствии с этим newLoadFactor.
    if (new_load_factor > max_load_factor) {
        return;
    }
    // Если мы сюда попали, значит нужно уменьшить количество корзин
    // Сделаем сначала reHash от нового количества корзин, чтобы
    // чтобы при resize ничего не потерять

    // Захватим все мьютексы
    bool flag = lock_all_mutexes();
    if (flag) {
        // Если успешно захватили, то рехэш и ресайз.
        rehash(new_buckets_num);
        // И, теперь уменьшим количество корзин
        buckets.resize(new_buckets_num);
        // Ну и отпустим все мьютексы.
        unlock_all_mutexes();
    }
}


template<class T, class H>
void striped_hash_set::resize(int new_buckets_num) {
    bool is_locked = lock_all_mutexes();
    if (is_locked) {
        unsigned long newBucketsNumber = growth_factor * this->buckets.size();
        buckets.resize(newBucketsNumber);
        rehash(newBucketsNumber);
        unlock_all_mutexes();
    }
}

template<class T, class H>
bool striped_hash_set::lock_all_mutexes() {
    return false;
}

template<class T, class H>
void striped_hash_set::rehash(unsigned long i) {
    // Пробежимся по всем корзинам и тупо по всем спискам,
    // перехешировывая всё что только можно.

    // По всем корзинам
    for(int i = 0; i < buckets.size(); ++i) {
        // По всем элементам корзины
        for(auto it = buckets[i].before_begin(); it._M_next() != buckets[i].end();) {
            // Поскольку список односвязный, и удалять элементы мы можем
            // только перед собой, то нужно проверить, не последние ли мы
            // и вообще, есть ли кто-то перед нами.

            //Если нет - пересчитываем хэш, нашу новую корзину
            unsigned int new_hash = H(*(it._M_next()));
            unsigned long new_bucket = new_hash % this->buckets.size();
            // Если корзина изменилась, то вставляем туда, где ему место
            // и удаляем отсюда
            if (new_bucket != i) {
                // Если хэш у элемента после нас изменился, то положим его туда,
                // где ему место, а здесь его удалим.
                buckets[new_bucket].push_front(*(it._M_next()));
                buckets[i].erase_after(it);
            } else {
                // Так как ничего в списке не перед нами не удалилось,
                // нужно продвинуться вперед.
                it++;
            }
        };
    };

}

template<class T, class H>
void striped_hash_set::unlock_all_mutexes() {
    for (auto it : stripe_locks) {
        it.write_unlock();
    }
}

void striped_hash_set::expand() {
        // Проверим, нужно ли вообще расширяться
        // с помощью loadFactor.
    if ( double(size) / buckets.size() < max_load_factor) {
        return;
    }
        // Если получилось захватить первый мьютекс
        // так, что никто не успел расширить
        // контейнер до нас, то расширять придётся
        // как раз нам.
        bool is_locked = lock_all_mutexes();
        if (is_locked) {
            unsigned long new_buckets_num = growth_factor * this->buckets.size();
            buckets.resize(new_buckets_num);
            // Затем нужно перехешироваться.
            rehash(new_buckets_num);
            // Отпустим все мьютексы.
            unlock_all_mutexes();
        }
}


template<class T, class H>
bool striped_hash_set<T, H>::lock_all_mutexes(){
    unsigned long old_buckets_number = this->buckets.size();
    // Захватили первый мьютекс.
    stripe_locks[0].write_lock();
    // Проверили.
    if (old_buckets_number!= this->buckets.size()) {
        stripe_locks[0].write_unlock();
        return false;
    }
    // Если размер остался тем, же то "дозахватим" остальные.
    for (int i = 1; i < stripe_locks.size(); ++i)
        stripe_locks[i].write_lock();
    // Расширяться/сжиматься нужно(никто не расширил до нас)!
    return true;
}

template<class T, class H>
void striped_hash_set<T, H>::rehash(unsigned long buckets_number) {
    // Пробежимся по всем корзинам и тупо по всем спискам,
    // перехешировывая всё что только можно.

    // По всем корзинам
    for(int i = 0; i < buckets.size(); ++i) {
        // По всем элементам корзины
        for(auto it = buckets[i].before_begin(); it._M_next() != buckets[i].end();) {
            // Поскольку список односвязный, и удалять элементы мы можем
            // только перед собой, то нужно проверить, не последние ли мы
            // и вообще, есть ли кто-то перед нами.

            //Если нет - пересчитываем хэш, нашу новую корзину
            unsigned int new_hash = H(*(it._M_next()));
            unsigned long new_bucket = new_hash % buckets_number;
            // Если корзина изменилась, то вставляем туда, где ему место
            // и удаляем отсюда
            if (new_bucket != i) {
                // Если хэш у элемента после нас изменился, то положим его туда,
                // где ему место, а здесь его удалим.
                buckets[new_bucket].push_front(*(it._M_next()));
                buckets[i].erase_after(it);
            } else {
                // Так как ничего в списке не перед нами не удалилось,
                // нужно продвинуться вперед.
                it++;
            }
        };
    };
}






