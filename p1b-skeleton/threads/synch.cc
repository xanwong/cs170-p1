// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Your solution for Task 2
// TODO

//----------------------------------------------------------------------
// Lock::Lock
// 	Initialize a lock to be used for mutual exclusion. The lock is initially free.
// debugName is an arbitrary name that is used for debugging.
// value=1 means the lock in FREE, value=0 means the lock is BUSY.
//----------------------------------------------------------------------

Lock::Lock(char* debugName) {
    name = debugName;
    value = 1;      // Lock is FREE
    owner = NULL;
    queue = new List;
}

//----------------------------------------------------------------------
// to deallocate lock when it is not needed anymore
//----------------------------------------------------------------------

Lock::~Lock() { 
    delete queue;
}

//----------------------------------------------------------------------
// Lock::Acquire
// wait until the lock is available, and then set to BUSY
// Record the current thread as the lock owner.
// Interrupts are disabled during checking the lock status.
// The current thread is added to the wait queue and put to sleep if the lock is BUSY.
//----------------------------------------------------------------------

void Lock::Acquire() { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    ASSERT(!isHeldByCurrentThread());       // Ensure the current thread doesn't already hold the lock to prevent deadlock
    while (value == 0) { 			// Lock is BUSY
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value = 0; 					// Lock available, set it to BUSY
    owner = currentThread;

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Lock::Release
// Releases the lock. The thread that acquired the lock is the one that releases it.
// Release sets the lock to FREE and wakes up a waiter from the waiting queue.
//----------------------------------------------------------------------

void Lock::Release() { 
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    ASSERT(isHeldByCurrentThread());        // Only the thread that holds the lock can release it

    value = 1; 					// Lock is now FREE
    owner = NULL;

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   
        scheduler->ReadyToRun(thread);  // Run next thread waiting for this lock
    
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
// Returns true if current thread holds the lock.
//----------------------------------------------------------------------

bool Lock::isHeldByCurrentThread() { 
    return owner == currentThread;
}

// Your solution for Task 3
// TODO

//----------------------------------------------------------------------
// Condition::Condition
// Initialize a condition variable, which has no value.
// Condition variable maintains queue of waiting threads.
//----------------------------------------------------------------------

Condition::Condition(char* debugName) { 
    name = debugName;
    queue = new List;  // this is the queue of threads that are waiting on this condition
}

//----------------------------------------------------------------------
// Condition::~Condition
// deallocates the condition variable when it is no longer needed.
//----------------------------------------------------------------------

Condition::~Condition() {
    delete queue;
}

//----------------------------------------------------------------------
// Condition::Wait
// Releases the lock and adds current thread to the waiting queue.
// Goes to sleep while waiting on condition. Acquire the lock again when woken.
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock) { 
    ASSERT(conditionLock->isHeldByCurrentThread());

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    queue->Append((void *)currentThread);
    
    conditionLock->Release();  // release the lock
    
    currentThread->Sleep();

    (void) interrupt->SetLevel(oldLevel);

    conditionLock->Acquire();
}

//----------------------------------------------------------------------
// Condition::Signal
// Wakes up one thread that is waiting on this condition.
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock) { 
    // caller must hold the lock
    ASSERT(conditionLock->isHeldByCurrentThread());

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    Thread *thread = (Thread *)queue->Remove();
    if(thread != NULL) {
        scheduler->ReadyToRun(thread);
    }

    (void) interrupt->SetLevel(oldLevel);  // re-enable interrupt
}

//----------------------------------------------------------------------
// Condition::Broadcast
// Wakes up all threads that are waiting on this condition variable.
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock) { 
    ASSERT(conditionLock->isHeldByCurrentThread());

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    Thread *thread;
    while((thread = (Thread *)queue->Remove()) != NULL) {
        scheduler->ReadyToRun(thread);
    }

    (void) interrupt->SetLevel(oldLevel);  //re-enable interrupt
}
