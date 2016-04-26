//
// Created by riv on 26.04.16.
//

#include "Semaphore.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX_RETRIES 10

union semun{
    int val;
    struct semid_ds *buf;
    ushort *array;
};

Semaphore::Semaphore(int nsems) {
    nSems = nsems;
    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;

    if((key = ftok("Semaphore.h", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    initSem();
}

int Semaphore::initSem(){
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;

    semid = semget(key, nSems, IPC_CREAT | IPC_EXCL | 0666);

    if(semid >= 0) {
        sb.sem_op = 1;
        sb.sem_flg = 0;
        arg.val = 1;

        for(sb.sem_num = 0; sb.sem_num < nSems; sb.sem_num++) {
            if(semop(semid, &sb, 1) == -1) {
                int e = errno;
                semctl(semid, 0, IPC_RMID);
                errno = e;
                return -1;
            }
        }
    }
    else if(errno == EEXIST) {
        int ready = 0;

        semid = semget(key, nSems, 0);

        if(semid < 0)
            return semid;

        arg.buf = &buf;

        for(i = 0; i < MAX_RETRIES && !ready; i++) {
            semctl(semid, nSems - 1, IPC_STAT, arg);
            if(arg.buf->sem_otime != 0) {
                ready = 1;
            }
            else {
                sleep(1);
            }
        }
        if(!ready) {
            errno = ETIME;
            return -1;
        }
    } else {
        return semid;
    }

    return semid;
}

void Semaphore::wait() {
    if(semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void Semaphore::signal() {
    sb.sem_op = 1;

    if(semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}