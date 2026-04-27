#include<stdio.h>
#include <pthread.h>
#include <iostream> 


#include "rwlock.h"

//Your solution to implement each of the following methods
//
RWLock::RWLock() { 
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&readers_ok, NULL);
    pthread_cond_init(&writers_ok, NULL);
    numReaders = 0;
    numWritersWaiting = 0;
    writer_active = false;
}

RWLock::~RWLock() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&readers_ok);
    pthread_cond_destroy(&writers_ok);
}

void RWLock::startRead() { 
    pthread_mutex_lock(&mutex);

    // readers only need to wait for writers to complete,
    // readers can read concurrently with other readers
    while (writer_active || numWritersWaiting > 0) {
        pthread_cond_wait(&readers_ok, &mutex);
    }
    numReaders++;

    pthread_mutex_unlock(&mutex);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&mutex);

    numReaders--;
    // signal to writers that they can start writing if there are no more readers
    if (numReaders == 0) {
        pthread_cond_signal(&writers_ok);
    }

    pthread_mutex_unlock(&mutex);
 }

void RWLock::startWrite() { 
    pthread_mutex_lock(&mutex);

    numWritersWaiting++;
    // have to wait for all readers to finish and for any active writer to finish
    while ( numReaders > 0  || writer_active ) {
        pthread_cond_wait(&writers_ok, &mutex);
    }
    numWritersWaiting--;
    writer_active = true;

    pthread_mutex_unlock(&mutex);
}

void RWLock::doneWrite() {
    pthread_mutex_lock(&mutex);

    writer_active = false;
    if (numWritersWaiting > 0) {
        pthread_cond_signal(&writers_ok);       // wake one writer
    } else {
        pthread_cond_broadcast(&readers_ok);    // wake all readers when no writers are waiting
    }
    pthread_mutex_unlock(&mutex);
 }
