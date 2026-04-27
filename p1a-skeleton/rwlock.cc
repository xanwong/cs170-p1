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
    activeReaders = 0;
    waitingReaders = 0;
    activeWriters = 0;
    waitingWriters = 0;
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
    while ( (activeWriters + waitingWriters) > 0) {
        waitingReaders++;
        pthread_cond_wait(&readers_ok, &mutex);
        waitingReaders--;
    }
    activeReaders++;

    pthread_mutex_unlock(&mutex);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&mutex);

    activeReaders--;
    // signal to writers that they can start writing if there are no more readers
    if ( activeReaders == 0 && waitingWriters > 0) {
        pthread_cond_signal(&writers_ok);
    }

    pthread_mutex_unlock(&mutex);
 }

void RWLock::startWrite() { 
    pthread_mutex_lock(&mutex);

    // have to wait for all readers to finish and for any active writer to finish
    while ( activeReaders > 0  || activeWriters > 0 ) {
        waitingWriters++;
        pthread_cond_wait(&writers_ok, &mutex);
        waitingWriters--;
    }
    activeWriters++;

    pthread_mutex_unlock(&mutex);
}

void RWLock::doneWrite() {
    pthread_mutex_lock(&mutex);

    activeWriters--;
    if (waitingWriters > 0) {
        pthread_cond_signal(&writers_ok);       // wake one writer
    } else {
        pthread_cond_broadcast(&readers_ok);    // wake all readers when no writers are waiting
    }
    pthread_mutex_unlock(&mutex);
 }
