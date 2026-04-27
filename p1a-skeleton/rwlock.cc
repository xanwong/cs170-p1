#include<stdio.h>
#include <pthread.h>
#include <iostream> 


#include "rwlock.h"

//Your solution to implement each of the following methods
//
RWLock::RWLock() { }
RWLock::~RWLock() { }
void RWLock::startRead() { }
void RWLock::doneRead() { }
void RWLock::startWrite() { }
void RWLock::doneWrite() { }
