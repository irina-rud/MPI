//
// Created by riv on 26.04.16.
//

#ifndef ROBOT_SEMAPHORE_H
#define ROBOT_SEMAPHORE_H

#include <sys/types.h>
#include <sys/sem.h>

class Semaphore
{
private:
    key_t key;
    int nSems;
    struct sembuf sb;
    int semid;

    int initSem();

public:
    Semaphore(int);

    void wait();

    void signal();
};

#endif //ROBOT_SEMAPHORE_H
