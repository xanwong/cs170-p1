#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include<semaphore.h>

class RWLock{
private:
    //Your solution to add more private fields
    int activeReaders;
    int waitingReaders;
    int activeWriters;
    int waitingWriters;

    pthread_mutex_t mutex;
    pthread_cond_t readers_ok;
    pthread_cond_t writers_ok;
    
public:
    RWLock();
    ~RWLock();
    //Reader
    void startRead();
    void doneRead();
    // Writer
    void startWrite();
    void  doneWrite();
};

#endif
