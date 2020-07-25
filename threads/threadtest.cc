// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "list.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;
Thread *thread[128];

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
        printf("*** thread named %s UserId %d ThreadId %d Priority %d looped %d times\n", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(), num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");
    Thread *t = new Thread("forked thread", 8);

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
    
}

//----------------------------------------------------------------------
// TS_GetInfromation
//  Print information and status of all the thread.
//----------------------------------------------------------------------

void
TS_GetInfromation(int arg)
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);

    // print information of current thread
    printf("Name: \"%s\"     ThreadID: %d     Status: %s \n", 
            currentThread->getName(), currentThread->getThreadId(), 
            ThreadStatusInChar[currentThread->getStatus()]);  
    
    List *list = new List;

    list = scheduler->getReadyList();

    if(!list->IsEmpty())
        list->Mapcar(TS_Print);   // print ready thread in the queue*/
    printf("\n");

    currentThread->Yield();
    interrupt->SetLevel(oldlevel);
}

//----------------------------------------------------------------------
// TS_Print
//  Print information and status of all the thread.
//----------------------------------------------------------------------
void
TS_Print(int arg)
{
    Thread *t = (Thread *)arg;
    printf("Name: \"%s\"     ThreadID: %d     Status: %s \n", 
            t->getName(), t->getThreadId(), ThreadStatusInChar[t->getStatus()]);  // print information 

}

//----------------------------------------------------------------------
// ThreadTest3
// Print whole information and status of current threads.
//----------------------------------------------------------------------

void
ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3:TS");
    
    printf("Current thread contents:\n");
    Thread *t1 = new Thread("Forked", 8);
    Thread *t2 = new Thread("Forked", 8);
    Thread *t3 = new Thread("Forked", 8);
     
    t1->Fork(TS_GetInfromation, 0);
    t2->Fork(TS_GetInfromation, 0);
    t3->Fork(TS_GetInfromation, 0);
}

//----------------------------------------------------------------------
// ThreadNumTest
//  Check whether the number of all the forked threads 
//  is over limit count(128).
//----------------------------------------------------------------------

void
ThreadNumTest()
{
    DEBUG('t', "Entering ThreadTest:Thread Number Limit");

    // try to fork 129 threads
    for (int i = 0; i < MaxThreadNum + 1; ++i) {
        Thread *t = new Thread("forked", 8);
        printf("thread name = %s, userId = %d, threadId = %d\n", 
                t->getName(), t->getUserId(), t->getThreadId());
    }
}

//----------------------------------------------------------------------
// ThreadTest4_1 - ThreadTest4_3:
// Test the priority of threads.
//----------------------------------------------------------------------

void
ThreadTest4_3(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
        printf("*** thread named %s UserId %d ThreadId %d Priority %d looped %d times\n", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(), num);
        if(num == 0){
            Thread *t4 = new Thread("T4", which);
            t4->Fork(SimpleThread, 0);
        }
    }
}

void
ThreadTest4_2(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
        printf("*** thread named %s UserId %d ThreadId %d Priority %d looped %d times\n", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(), num);
        if(num == 0){
            Thread *t3 = new Thread("T3", which);
            t3->Fork(ThreadTest4_3, 2);
        }
    }
}

void
ThreadTest4_1(int which)
{
    int num;
    for (num = 0; num < 5; num++) {
        printf("*** thread named %s UserId %d ThreadId %d Priority %d looped %d times\n", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(), num);
        if(num == 0){
            Thread *t2 = new Thread("T2", which);
            t2->Fork(ThreadTest4_2, 3);
        }
    }
}

//----------------------------------------------------------------------
// ThreadTest4
// Test the priority of threads.
//----------------------------------------------------------------------

void
ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4:");
    
    printf("Current thread contents:\n");
    Thread *t1 = new Thread("T1", 9);
     
    t1->Fork(ThreadTest4_1, 1);
}

//----------------------------------------------------------------------
// TimeOneTick
// Make time run.
//----------------------------------------------------------------------

void TimeOneTick(){
    interrupt->SetLevel(IntOn);
    interrupt->SetLevel(IntOff);
}

//----------------------------------------------------------------------
// ThreadTest5_1
// Test the time slices transfer.
//----------------------------------------------------------------------

void
ThreadTest5_1(int fromt4)
{
    for(int num = 0;num < 10;++num){
        int timeslices = currentThread->getTimeSlices();
        printf("*** thread named %s UserId %d ThreadId %d Priority %d ListNumber %d looped %d times, timeslices:%d\n", \
            currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId(),\
            currentThread->getPriority(), currentThread->getListNumber(), \
            num, timeslices % LimitTimeSlice);
        TimeOneTick(); 
        if(num == 0 && fromt4 == 0){
            Thread *t4 = new Thread("T4", 1);
            t4->Fork(ThreadTest5_1, TRUE);
        }
    }
}

//----------------------------------------------------------------------
// ThreadTest5
// Test the time slices transfer.
//----------------------------------------------------------------------

void
ThreadTest5()
{
    DEBUG('t', "Entering ThreadTest5:");
    
    printf("Current thread contents:\n");
    Thread *t1 = new Thread("T1", 9);
    Thread *t2 = new Thread("T2", 2);
    Thread *t3 = new Thread("T3", 1);
     
    t1->Fork(ThreadTest5_1, 0);
    t2->Fork(ThreadTest5_1, 0);
    t3->Fork(ThreadTest5_1, 0);
}

//----------------------------------------------------------------------
// TestProCon:
// Used to test the Producer-Consumer model.
// Contain a lock, two conditions of both producer and consumer.
//----------------------------------------------------------------------
class TestProCon{
    public:
        int CurNum;
        int MaxNum;
        List* items;
        Lock* lock;
        Condition* ConProd;
        Condition* ConCons;
    public:
        TestProCon()
        {
            CurNum = 0;
            MaxNum = 10;
            items = new List;
            lock = new Lock("ProducerConsumerLock");
            ConProd = new Condition("ConProd");
            ConCons = new Condition("Concons");
        }
        ~TestProCon() { delete items; delete lock; }
        void CompleteProduce() { items->Append(&CurNum); }
        void* ConsumeItem() { return items->Remove(); }
};

//----------------------------------------------------------------------
// Initialize tpr to test the Producer-Consumer model.
//----------------------------------------------------------------------
TestProCon* tpr = new TestProCon();

Semaphore* Empty = new Semaphore("Empty", 10); // whether the list is empty
Semaphore* Mutex = new Semaphore("Lock", 1);    // set the mutex
Semaphore* Full = new Semaphore("Full", 0);     // whether list is full
int ProductNumber = 0;

//----------------------------------------------------------------------
// Producer:
// Do the producer's work.
// Uses lock and conditions.
//----------------------------------------------------------------------
void Producer(int num){
    for(int i = 0;i < num;++i){
        tpr->lock->Acquire();

        while(tpr->CurNum >= tpr->MaxNum){
            printf("Product List Is Full! Waiting For Customer...\n");
            tpr->ConProd->Wait(tpr->lock);
        }
        tpr->CompleteProduce();
        printf("Produce item %d into the list.\n", tpr->CurNum++);
        tpr->ConCons->Signal(tpr->lock);

        tpr->lock->Release();
    }
}

//----------------------------------------------------------------------
// Consumer:
// Do the consumer's work.
// Uses lock and conditions.
//----------------------------------------------------------------------

void Consumer(int num){
    for(int i = 0;i < num;++i){
        tpr->lock->Acquire();

        while(tpr->CurNum <= 0){
            printf("Product List Is Empty! Waiting For Producer...\n");
            tpr->ConCons->Wait(tpr->lock);
        }
        tpr->ConsumeItem();
        printf("Consume item %d in the list.\n", tpr->CurNum--);
        tpr->ConProd->Signal(tpr->lock);

        tpr->lock->Release();
    }
}

void Producer_Sem(int num){
    for(int i = 0;i < num;++i) {
        Empty->P();
        Mutex->P();
        printf("Produce item %d into the list.\n", ProductNumber++);
        Mutex->V();
        Full->V();
    }
}

void Consumer_Sem(int num){
    for(int i = 0;i < num;++i){
        Full->P();
        Mutex->P();
        printf("Consume item %d in the list.\n", --ProductNumber);
        Mutex->V();
        Empty->V();
    }
}

//----------------------------------------------------------------------
// ProConTest:
// Test the Producer-Consumer model.
//----------------------------------------------------------------------

void
ProConTest()
{
    DEBUG('t', "Entering ThreadTest6:");
    
    Thread *t1 = new Thread("Producer", 1);
    Thread *t2 = new Thread("Consumer", 1);
     
    t1->Fork(Producer_Sem, 23);
    t2->Fork(Consumer_Sem, 15);
}

int Data = 0;    // content to read
int ReadNum = 0;    // number of reader
// locks for reader and writer
Lock* ReadLock = new Lock("ReadLock");
Lock* WriteLock = new Lock("WriteLock");

void Reader(int num){
    for(int i = 0;i < num;++i){
        ReadLock->Acquire();
        ReadNum++;
        if(ReadNum == 1)
            WriteLock->Acquire();
        ReadLock->Release();
        if(Data == 0)
            printf("No Data To Read!\n");
        else
            printf("Thread %s Read Data. The Value is %d.\n",\
                    currentThread->getName(), Data);
        ReadLock->Acquire();
        ReadNum--;
        if(ReadNum == 0)
            WriteLock->Release();
        ReadLock->Release();
        currentThread->Yield();
    }
}

void Writer(int num){
    for(int i = 0;i < num;++i){
        WriteLock->Acquire();
        Data++;
        printf("Thread %s Write Data. Old Value is %d. New Value is %d.\n",\
                currentThread->getName(), Data-1, Data);
        WriteLock->Release();
        currentThread->Yield();
    }
}

void ReaWriTest()
{
    DEBUG('t', "Entering ReaWriTest:\n");
    
    Thread *r1 = new Thread("Reader 1", 1);
    Thread *r2 = new Thread("Reader 2", 1);
    Thread *r3 = new Thread("Reader 3", 1);
    Thread *w1 = new Thread("Writer 1", 1);
    Thread *w2 = new Thread("Writer 2", 1);
     
    r1->Fork(Reader, 20);
    w1->Fork(Writer, 20);
    r2->Fork(Reader, 20);
    w2->Fork(Writer, 20);
    r3->Fork(Reader, 20);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:              // original test
        ThreadTest1();
        break;
    case 2:              // test total thread number
        ThreadNumTest();
        break;
    case 3:              // "TS":print thread information and status
        ThreadTest3();
        break;
    case 4:
        ThreadTest4();
        break;
    case 5:
        ThreadTest5();
        break;
    case 6:
        ProConTest();
        break;
    case 7:
        ReaWriTest();
        break;
    default:
	    printf("No test specified.\n");
	    break;
    }
}

