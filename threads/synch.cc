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
    //printf("value=0\n");
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

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    name = debugName;
    SemaphoreOfLock = new Semaphore(name, 1);
    OwnerThread = NULL;
}
Lock::~Lock() {
    delete SemaphoreOfLock;
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread()
// Return true if the current threadholds this lock.  
// Useful for checking in Release, and in condition variable ops below.
//----------------------------------------------------------------------
bool Lock::isHeldByCurrentThread() {
    return (currentThread == OwnerThread);
}

void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
   
    SemaphoreOfLock->P();
    OwnerThread = currentThread;

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    ASSERT(this->isHeldByCurrentThread());
    
    SemaphoreOfLock->V();
    OwnerThread = NULL;
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}


Condition::Condition(char* debugName) { 
    name = debugName;
    queue = new List;
}

Condition::~Condition() {
    delete queue;
}

//----------------------------------------------------------------------
// Condition::Wait(Lock* conditionLock) 
// These are the 3 operations on condition variables:
// - Releasing the lock 
// - Going to sleep
// - Acquire the lock
//----------------------------------------------------------------------
void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    // if the thread is the one which own the lock
    ASSERT(conditionLock->isHeldByCurrentThread());     

    conditionLock->Release();         // release the lock
    queue->Append(currentThread);     // append current thread into the waiting queue
    currentThread->Sleep();           
    conditionLock->Acquire();         // acquire the lock
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Condition::Signal(Lock* conditionLock) 
// Make next waiting thread ready, and let it hold the conditonLock.
//----------------------------------------------------------------------
void Condition::Signal(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    // if the thread is the one which own the lock
    ASSERT(conditionLock->isHeldByCurrentThread());     

    if(!queue->IsEmpty()) {
        // woke next waiting thread up, make it Ready
        Thread* NextToRun = (Thread*)queue->Remove();
        scheduler->ReadyToRun(NextToRun);
    }

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Condition::Broadcast(Lock* conditionLock)
// Wake up all the waiting threads for this condition.
//----------------------------------------------------------------------
void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    // if the thread is the one which own the lock
    ASSERT(conditionLock->isHeldByCurrentThread());     

    while(!queue->IsEmpty()) {
        Signal(conditionLock);
    }

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
