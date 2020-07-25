// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H


#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

#define MaxMessNum 128 * 4

//#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
//#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#include "synchdisk.h"   //lab6需要
extern SynchDisk   *synchDisk;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

class MessageQueue{
  public:
    bool valid;
    int dest;
    char* Mess;
};

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.
extern int Send(int dest, char* data);
extern int Receive(int dest, char* content);

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock
extern int MaxThreadNum;			// set the max thread number
extern int ThreadIdArray[128];		// thread id array
extern MessageQueue mess[MaxMessNum];

#endif // SYSTEM_H
