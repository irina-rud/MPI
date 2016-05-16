//
// Created by riv on 29.04.16.
//

#include <list>
#include "striped_hash_set.h"

template<typename T, class H = std::hash<T>>
striped_hash_set::striped_hash_set(int num_stripes = 5 , double growth_factor = 2, double max_load_factor = 2){
    this->num_stripes = num_stripes;
    this->growth_factor = growth_factor;
    this->max_load_factor = max_load_factor;
    this->size = 0;
    this->stripe_locks.resize(num_stripes);
    this->buckets.resize(num_stripes);
}

template<typename T, class H = std::hash<T>>
bool striped_hash_set::contains(const T &element) const{
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

}

template<typename T, class H = std::hash<T>>
void striped_hash_set::add(const T &element) {
    if (this->contains(element)) {
        return;
    }
    if (this->)
    resize();
    unsigned int witch_back = H(element) % this->buckets.size();
    unsigned int witch_mutex = (unsigned int) (witch_back % this->stripe_locks.size());
    stripe_locks[witch_mutex].write_lock();
    buckets[witch_back].push_front(element);
    stripe_locks[witch_mutex].write_lock();
    std::unique_lock<std::mutex> unic_resize_lock(size_lock);
        this->size++;
}

void striped_hash_set::resize() {
    bool toBeContinued = smartlyGrabAllMutexes();
    if (!toBeContinued) {
        return;
    }
    // Расширимся...
    unsigned int newBucketsNumber = growthFactor_ * getCurrentBucketsNumber();
    buckets_.resize(newBucketsNumber);
    // Затем нужно перехешироваться.
    reHash(newBucketsNumber);
    // Отпустим все мьютексы.
    releaseAllMutexes();
}


template<class T, class H>
bool ThreadSafeHashSet<T, H>::smartlyGrabAllMutexes() {
    // Запомним перед количество корзин,
    // захватим один мьютекс, и, если окажется,
    // что количество корзин изменилось, то значит кто-то успел расширить/сжать
    // наш контейнер до нас. Бросим это занятие.
    //
    // Достаточно захватить один мьютекс, чтобы
    // предотвратить двукратное расширение/сужение контейнера.

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

    // Запомнили количество корзин.
    unsigned int oldBucketsNumber = getCurrentBucketsNumber();
    // Захватили первый мьютекс.
    stripesLocks_[0].writeLock();
    // Проверили.
    if (oldBucketsNumber!= getCurrentBucketsNumber()) {
        // Расширяться/сжиматься не нужно(кто-то уже расширил/сжал до нас)!
        stripesLocks_[0].writeUnlock();
        return false;
    }
    // Если размер остался тем, же то "дозахватим" остальные.
    for (int i = 1; i < stripesLocks_.size(); ++i)
        stripesLocks_[i].writeLock();
    // Расширяться/сжиматься нужно(никто не расширил до нас)!
    return true;
}

template<class T, class H>
void ThreadSafeHashSet<T, H>::reHash(unsigned int bucketsNumber) {
    // Пробежимся по всем корзинам и тупо по всем спискам,
    // перехешировывая всё что только можно.

    // По всем корзинам
    for(int i = 0; i < buckets_.size(); ++i) {
        // По всем элементам корзины
        for(auto it = buckets_[i].before_begin(); it._M_next() != buckets_[i].end();) {
            // Поскольку список односвязный, и удалять элементы мы можем
            // только перед собой, то нужно проверить, не последние ли мы
            // и вообще, есть ли кто-то перед нами.

            //Если нет - пересчитываем хэш, нашу новую корзину
            unsigned int newHash = getHash(*(it._M_next()));
            unsigned int newBucket = newHash % bucketsNumber;
            // Если корзина изменилась, то вставляем туда, где ему место
            // и удаляем отсюда
            if (newBucket != i) {
                // Если хэш у элемента после нас изменился, то положим его туда,
                // где ему место, а здесь его удалим.
                buckets_[newBucket].push_front(*(it._M_next()));
                buckets_[i].erase_after(it);
            } else {
                // Так как ничего в списке не перед нами не удалилось,
                // нужно продвинуться вперед.
                it++;
            }
        };
    };
}






