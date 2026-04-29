#include<stdio.h>
#include <iostream> //


#include "rwlock.h"
#include "synch.h"

//Implement all methods of RWLock defined in rwlock.h
//Your solution for  Task 3 
//TODO

//----------------------------------------------------------------------
// RWLock::RWLock
// initialize the state counters to 0 and create variables.
//----------------------------------------------------------------------

RWLock::RWLock(char *debugName){ 
    AR = 0; // no active readers
    AW = 0; // no active writers
    WR = 0; // no waiting readers
    WW = 0; // no waiting writers

    lock = new Lock(debugName);

    // condition variables
    condition_read = new Condition(debugName);
    condition_write = new Condition(debugName);
}

//----------------------------------------------------------------------
// RWLock::~RWLock()
// releases the condition variables and monitor lock.
//----------------------------------------------------------------------

RWLock::~RWLock(){ 
    delete lock;
    delete condition_read;
    delete condition_write;
}

//----------------------------------------------------------------------
// RWLock::startRead
// reader waits if any writer is active or waiting.
//----------------------------------------------------------------------

void RWLock::startRead(){ 
    lock->Acquire();

    // wait if there is any writer that is active or waiting
    while ((AW + WW) > 0) {
        WR++;
        condition_read->Wait(lock);
        WR--;
    }

    AR++;
    lock->Release();
}

//----------------------------------------------------------------------
// RWLock::doneRead
// decrement number of active readers AR.
//----------------------------------------------------------------------

void RWLock::doneRead(){ 
    lock->Acquire();
    AR--;

    if(AR == 0 && WW > 0) {
        condition_write->Signal(lock);
    }

    lock->Release();
}

//----------------------------------------------------------------------
// RWLock::startWrite
// writer waits until no readers and no writers are active.
//----------------------------------------------------------------------

void RWLock::startWrite(){ 
    lock->Acquire();

    while((AW + AR) > 0) {
        WW++;
        condition_write->Wait(lock);
        WW--;
    }

    AW++;
    lock->Release();
}

//----------------------------------------------------------------------
// RWLock::doneWrite
// decrement number of active writers AW. Decides who to wake up.
//----------------------------------------------------------------------

void RWLock::doneWrite(){ 
    lock->Acquire();

    AW--;

    if(WW > 0) {
        condition_write->Signal(lock);
    }
    else if(WR > 0) {
        condition_read->Broadcast(lock);
    }

    lock->Release();

}
