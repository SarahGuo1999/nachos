// synchdisk.cc 
//	Routines to synchronously access the disk.  The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchdisk.h"
#include "system.h"
#include "thread.h"

//----------------------------------------------------------------------
// DiskRequestDone
// 	Disk interrupt handler.  Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
DiskRequestDone (int arg)
{
    SynchDisk* disk = (SynchDisk *)arg;

    disk->RequestDone();
}

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//	"name" -- UNIX file name to be used as storage for the disk data
//	   (usually, "DISK")
//----------------------------------------------------------------------

SynchDisk::SynchDisk(char* name)
{
    semaphore = new Semaphore("synch disk", 0);
    lock = new Lock("synch disk lock");
    Numlock = new Lock("Reader Numlock");
    disk = new Disk(name, DiskRequestDone, (int) this);
    for(int i = 0;i < NumSectors;++i){
        mutex[i] = new Semaphore("name",1);
        VisitNum[i] = 0;
    }
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
// 	De-allocate data structures needed for the synchronous disk
//	abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk()
{
    delete disk;
    delete lock;
    delete semaphore;
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
// 	Read the contents of a disk sector into a buffer.  Return only
//	after the data has been read.
//
//	"sectorNumber" -- the disk sector to read
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{ 
    lock->Acquire();			// only one disk I/O at a time
    disk->ReadRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt
    //printf("Phou\n");
    lock->Release();
    //printf("release后\n");
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"sectorNumber" -- the disk sector to be written
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{
    lock->Acquire(); 			// only one disk I/O at a time
    disk->WriteRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::RequestDone
// 	Disk interrupt handler.  Wake up any thread waiting for the disk
//	request to finish.
//----------------------------------------------------------------------
 
void
SynchDisk::RequestDone()
{ 
    semaphore->V();
}

void
SynchDisk::BeginRead(int sector){ 
    printf("read\n");
    Numlock->Acquire();
    ++ReaderCnt[sector];
    if(ReaderCnt[sector] == 1)
        mutex[sector]->P();
    printf("Thread:%s . %d readers reading sector: %d\n",currentThread->getName(), ReaderCnt[sector], sector);
    Numlock->Release();
}

void 
SynchDisk::EndRead(int sector){
    Numlock->Acquire();
    --ReaderCnt[sector];
    if(ReaderCnt[sector] == 0)
        mutex[sector]->V();
    Numlock->Release();
}

void 
SynchDisk::BeginWrite(int sector){ 
    //printf("Num:%d sector:%d\n",NumSectors,sector); 
    mutex[sector]->P();
}



    