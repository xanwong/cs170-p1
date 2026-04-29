/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/

#ifndef HASHCHAIN_H
#define HASHCHAIN_H

#include "synch.h"

#ifdef P1_RWLOCK
#include "rwlock.h"
#endif

#define TABLE_SIZE 128
class LinkedHashEntry {
private:
      int key;
      int value;
      LinkedHashEntry *next;
public:
      LinkedHashEntry(int key, int value); 
      int getKey(); 
      int getValue();
      void setValue(int value);
 
      LinkedHashEntry *getNext(); 
      void setNext(LinkedHashEntry *next); 
};


class HashMap {
private:
      LinkedHashEntry **table;

      int _get(int key);
      void _put(int key, int value);

#ifdef P1_SEMAPHORE
      Semaphore  *sem[TABLE_SIZE];
#elif defined P1_LOCK
      Lock  *locks[TABLE_SIZE];
#elif  defined P1_RWLOCK
      RWLock *rwlocks[TABLE_SIZE];	
#endif
      void _put(int key, int value);
      int _get(int key);

public:
      HashMap(); 
      int get(int key); 
      void put(int key, int value); 
      void remove(int key); 
      void increment(int key, int value); //increase key by value (or init key to zero)
      ~HashMap(); 
};


#endif // HASHCHAIN_H