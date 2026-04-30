/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 * Use fine-grain rwlock or lock
 **/



#include <iostream>
#include <unistd.h>
//#include "rwlock.h"
#include "hashchain.h"
#include "synch.h"

//NOYIELD disables the yields used to help uncover synchronization bugs
//  - the yields are useful for uncovering bugs, but performance testing may be easier without them
//#define NOYIELD 1

#ifndef NOYIELD
#ifdef NACHOS
#include "system.h" //for currentThread->Yield()
#define YIELD() currentThread->Yield()
#else //else pthreads
#define YIELD() pthread_yield()
#endif
#else //else NOYIELD
#define YIELD() do{}while(0)
#endif

//You don't need to use these, but if you use these macros then you only need to insert one line of code at each synchronization point down in the main code
//You can define the behavior for each lock call like follows:
//  #define START_READ() lock.lock()
//  #define END_READ() lock.unlock()
#ifdef NOLOCK
//no synchronization code
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#elif defined P1_SEMAPHORE //using nachos semaphore. Your solution for Task 1
#define START_READ(hash) sem[hash]->P()
#define END_READ(hash) sem[hash]->V()
#define START_WRITE(hash) sem[hash]->P()
#define END_WRITE(hash) sem[hash]->V()
#elif defined P1_LOCK //using our implemented nachos lock. Your solution for Task 2
#define START_READ(hash) lck[hash]->Acquire()
#define END_READ(hash) lck[hash]->Release()
#define START_WRITE(hash) lck[hash]->Acquire()
#define END_WRITE(hash) lck[hash]->Release()
#elif defined P1_RWLOCK //using our rwlock. Your solution for Task 3
#define START_READ(hash) rwlck[hash]->startRead()
#define END_READ(hash) rwlck[hash]->doneRead()
#define START_WRITE(hash) rwlck[hash]->startWrite()
#define END_WRITE(hash) rwlck[hash]->doneWrite()
#else //else behave like NOLOCK (no option passed)
#define START_READ(hash) do{}while(0)
#define END_READ(hash) do{}while(0)
#define START_WRITE(hash) do{}while(0)
#define END_WRITE(hash) do{}while(0)
#endif

LinkedHashEntry:: LinkedHashEntry(int key1, int value1) {
  this->key = key1;
  this->value = value1;
  this->next = NULL;
}

int 
LinkedHashEntry:: getKey() {
  return key;
}
int 
LinkedHashEntry:: getValue() {
  return value;
}

void 
LinkedHashEntry:: setValue(int value1) {
  this->value = value1;
}


LinkedHashEntry * 
LinkedHashEntry:: getNext() {
  return next;
}

void 
LinkedHashEntry:: setNext(LinkedHashEntry *next1) {
  this->next = next1;
}

const int TABLE_SIZE = 128;

HashMap::HashMap() {
  table = new LinkedHashEntry*[TABLE_SIZE];
  for (int i = 0; i < TABLE_SIZE; i++)
    table[i] = NULL;
#ifdef P1_SEMAPHORE
  for (int i = 0; i < TABLE_SIZE; i++) {
    sem[i] = new Semaphore((char *)"hash_bucket_sem", 1);
  }
  //insert setup code here
#elif defined P1_LOCK
  for(int i = 0; i < TABLE_SIZE; i++) {
    lck[i] = new Lock((char *)"hash_bucket_lock");

    // allocate one lock per bucket, locks start FREE and enforce mutual exclusion
  }

  //insert setup code here
#elif defined P1_RWLOCK
  for(int i = 0; i < TABLE_SIZE; i++) {
    rwlck[i] = new RWLock((char *)"hash_bucket_rwlock");

    // allocate one rwlock per bucket
    // allows concurrent reads and exclusive writes
  }

  //insert setup code here
#endif
}

int 
HashMap::_get(int key) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    return -1;
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry != NULL && entry->getKey() != key) {
      entry = entry->getNext();
      YIELD();
    }
    if (entry == NULL) {
      YIELD();
      return -1;
    } else { 
      YIELD();
      return entry->getValue();
    }
  }
  return -1; //should never get here (just for complaining compilers)
}

void
HashMap::_put(int key, int value) { //internal put() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    table[hash] = new LinkedHashEntry(key, value);
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      entry->setValue(value);
    } else {
      YIELD();
      entry->setNext(new LinkedHashEntry(key, value));
    }
  }
  YIELD();
}

int 
HashMap::get(int key1) { 
  int hash = (key1 % TABLE_SIZE); 
  START_READ(hash);
  //usleep(10);
  int ret= _get(key1);;
  END_READ(hash);
  return ret;
}

void 
HashMap::put(int key1, int value1) { 
  int hash = (key1 % TABLE_SIZE); 
  START_WRITE(hash);
  _put(key1,value1);
  END_WRITE(hash);
}


void
HashMap::remove(int key) { 
  int hash = (key % TABLE_SIZE);
  START_WRITE(hash);
  if (table[hash] != NULL) {
    YIELD();
    LinkedHashEntry *prevEntry = NULL;
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      prevEntry = entry;
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      if (prevEntry == NULL) {
        LinkedHashEntry *nextEntry = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        table[hash] = nextEntry;
      } else {
        LinkedHashEntry *next = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        prevEntry->setNext(next);
      }
    }
  }
  END_WRITE(hash);
}


void
HashMap::increment(int key, int value) { 
  int hash = (key % TABLE_SIZE); 
  START_WRITE(hash);
  _put(key,_get(key)+value);
  END_WRITE(hash);
}


HashMap:: ~HashMap() {
  for (int hash = 0; hash < TABLE_SIZE; hash++){
    if (table[hash] != NULL) {
      LinkedHashEntry *prevEntry = NULL;
      LinkedHashEntry *entry = table[hash];
      while (entry != NULL) {
        prevEntry = entry;
        entry = entry->getNext();
        delete prevEntry;
      }
    }
  }
  delete[] table;

#ifdef P1_SEMAPHORE
  // clean up semaphores
  for(int i = 0; i < TABLE_SIZE; i++) {
    delete sem[i];
  }
#endif
#ifdef P1_LOCK
  // clean up locks
  for(int i = 0; i < TABLE_SIZE; i++) {
    delete locks[i];
  }
#endif
#ifdef P1_RWLOCK
  // clean up rwlocks
  for(int i = 0; i < TABLE_SIZE; i++) {
    delete rwlocks[i];
  }
#endif
}