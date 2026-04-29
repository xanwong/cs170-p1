#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include<semaphore.h>
#include "synch.h"

class RWLock{
private:
      //Add  fields for  RWLock. Your solution to Task 3
      //TODO

	  int AR;  // number of active readers
	  int AW;  // number of active writers
	  int WR; // number of waiting readers
	  int WW;  // number of waiting writers

	  Lock *lock;  // monitor lock that protects state variables

	  Condition *condition_read;  // condition variable for readers
	  Condition *condition_write;  // condition variable for writers

public:
    	RWLock(char *debugName);
    	~RWLock();
    //Reader
    	void startRead();
    	void doneRead();
    // Writer
    	void startWrite();
    	void  doneWrite();
};

#endif
