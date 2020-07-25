// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "filesys.h"
#include "openfile.h"

void 
func3(int arg)
{
    printf("Thread3 Run:\n");  
    machine->Run();
}

// void
// func2(int arg)
// {
//     OpenFile *executable3 = fileSystem->Open((char*)arg);
//     AddrSpace *space3;
//     if (executable3 == NULL) {
//         printf("Thread3 Unable to open file %s\n", (char*)arg);
//         return;
//     }
//     printf("Thread3 Memory Allocate:\n");
//     space3 = new AddrSpace(executable3); 
//     space3->InitRegisters();		// set the initial register values
//     space3->RestoreState();		// load page table register
//     Thread *thread = new Thread("Thread3",1);
//     thread->space = space3;
    
//     thread->Fork(func3, 1);
//     currentThread->Yield();
//     delete executable3;			// close file

//     printf("Thread2 Run:\n");  
//     machine->Run();			// jump to the user progam
//     ASSERT(FALSE);			
// }
// //----------------------------------------------------------------------
// // StartProcess
// // 	Run a user program.  Open the executable, load it into
// //	memory, and jump to it.
// //----------------------------------------------------------------------

// void
// StartProcess1(char *filename)
// {
//     OpenFile *executable1 = fileSystem->Open(filename);
//     OpenFile *executable2 = fileSystem->Open(filename);
//     AddrSpace *space1;
//     AddrSpace *space2;

//     if (executable1 == NULL) {
//         printf("Thread1 Unable to open file %s\n", filename);
//         return;
//     }
//     if (executable2 == NULL) {
//         printf("Thread2 Unable to open file %s\n", filename);
//         return;
//     }
//     printf("Thread1 Memory Allocate:\n");
//     space1 = new AddrSpace(executable1); 
//     printf("Thread2 Memory Allocate:\n");  
//     space2 = new AddrSpace(executable2);    
    
//     currentThread->space = space1;
//     space2->InitRegisters();		// set the initial register values
//     space2->RestoreState();		// load page table register
//     Thread *thread = new Thread("Thread2",2);
//     thread->space = space2;

//     thread->Fork(func2, (int)filename);
//     currentThread->Yield();
//     delete executable1;			// close file
//     delete executable2;			// close file

//     space1->InitRegisters();		// set the initial register values
//     space1->RestoreState();		// load page table register
    
//     printf("Thread1 Run:\n");  
//     machine->Run();			// jump to the user progam
//     ASSERT(FALSE);	
// }

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}

void
StartProcess(char *filename)
{
    OpenFile *executable;
    int fileDescriptor = OpenForReadWrite(filename, FALSE);

	if (fileDescriptor != -1)
	    executable = new OpenFile(fileDescriptor);
    else
        executable = NULL; 

    AddrSpace *space;

    if (executable == NULL) {
	    printf("Unable to open file %s\n", filename);
	    return;
    }
    space = new AddrSpace(executable);    
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

