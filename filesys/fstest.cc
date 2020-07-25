// fstest.cc 
//	Simple test routines for the file system.  
//
//	We implement:
//	   Copy -- copy a file from UNIX to Nachos
//	   Print -- cat the contents of a Nachos file 
//	   Perftest -- a stress test for the Nachos file system
//		read and write a really large file in tiny chunks
//		(won't work on baseline system!)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "utility.h"
#include "filesys.h"
#include "system.h"
#include "thread.h"
#include "disk.h"
#include "stats.h"

#define TransferSize 	10 	// make it small, just to be difficult

//----------------------------------------------------------------------
// Copy
// 	Copy the contents of the UNIX file "from" to the Nachos file "to"
//----------------------------------------------------------------------
#ifdef FILESYS

void
Copy(char *from, char *to)
{
    
    FILE *fp;
    OpenFile* openFile;
    int amountRead, fileLength;
    char *buffer;

// Open UNIX file
    if ((fp = fopen(from, "r")) == NULL) {	 
	printf("Copy: couldn't open input file %s\n", from);
	return;
    }

// Figure out length of UNIX file
    fseek(fp, 0, 2);		
    fileLength = ftell(fp);
    fseek(fp, 0, 0);

// Create a Nachos file of the same length
    DEBUG('f', "Copying file %s, size %d, to file %s\n", from, fileLength, to);
    if (!fileSystem->Create(to, fileLength)) {	 // Create Nachos file
        printf("Copy: couldn't create output file %s\n", to);
        fclose(fp);
        return;
    }
    
    
    openFile = fileSystem->Open(to);
    ASSERT(openFile != NULL);
    
// Copy the data in TransferSize chunks
    buffer = new char[TransferSize];
    while ((amountRead = fread(buffer, sizeof(char), TransferSize, fp)) > 0)
	openFile->Write(buffer, amountRead);
    fileSystem->Print();
    delete [] buffer;

// Close the UNIX and the Nachos files
    delete openFile;
    fclose(fp);
}

//----------------------------------------------------------------------
// Print
// 	Print the contents of the Nachos file "name".
//----------------------------------------------------------------------

void
Print(char *name)
{
    OpenFile *openFile;    
    int i, amountRead;
    char *buffer;

    if ((openFile = fileSystem->Open(name)) == NULL) {
	printf("Print: unable to open file %s\n", name);
	return;
    }
    
    buffer = new char[TransferSize];
    while ((amountRead = openFile->Read(buffer, TransferSize)) > 0)
	for (i = 0; i < amountRead; i++)
	    printf("%c", buffer[i]);
    delete [] buffer;

    delete openFile;		// close the Nachos file
    return;
}

//----------------------------------------------------------------------
// PerformanceTest
// 	Stress the Nachos file system by creating a large file, writing
//	it out a bit at a time, reading it back a bit at a time, and then
//	deleting the file.
//
//	Implemented as three separate routines:
//	  FileWrite -- write the file
//	  FileRead -- read the file
//	  PerformanceTest -- overall control, and print out performance #'s
//----------------------------------------------------------------------

#define FileName 	"TestFile.txt"
#define Contents 	"1234567890"
#define ContentSize 	strlen(Contents)
#define FileSize 	((int)(ContentSize * 40))

static void 
FileWrite()
{
    OpenFile *openFile;    
    int i, numBytes;

    /*printf("Sequential write of %d byte file, in %d byte chunks\n", 
	FileSize, ContentSize);*/
    printf("Thread:\"%s\" is writing.\n", currentThread->getName());  //test
   
    if (!fileSystem->Create(FileName, 150)) {
      printf("Perf test: can't create %s\n", FileName);
      //return;
    }
    openFile = fileSystem->Open(FileName);
    if (openFile == NULL) {
	printf("Perf test: unable to open %s\n", FileName);
	return;
    }
   
    for (i = 0; i < FileSize; i += ContentSize) {
        printf("写~\n");
        numBytes = openFile->Write(Contents, ContentSize);
        if (numBytes < 10) {
            //printf("numBytes:%d\n",numBytes);
            printf("Perf test: unable to write %s\n", FileName);
            delete openFile;
            return;
        }
    }
    delete openFile;	// close file
}

static void 
FileRead()
{
    OpenFile *openFile;    
    char *buffer = new char[ContentSize];
    int i, numBytes;

    printf("Thread:\"%s\" is reading.\n", currentThread->getName());  //test
    
    if ((openFile = fileSystem->Open(FileName)) == NULL) {
	printf("Perf test: unable to open file %s\n", FileName);
	delete [] buffer;
	return;
    }
    //printf("Sequential read of %d byte file, in %d byte chunks\n\n", 
	//FileSize, ContentSize);

    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Read(buffer, ContentSize);
	if ((numBytes < 10) || strncmp(buffer, Contents, ContentSize)) {
	    printf("Perf test: unable to read %s\n", FileName);
	    delete openFile;
	    delete [] buffer;
	    return;
	}
    }
    delete [] buffer;
    delete openFile;	// close file
    currentThread->Yield();
}

void
PerformanceTest()
{
    printf("Starting file system performance test:\n");
    stats->Print();
    printf("\n");
    FileWrite();
    FileRead();
    if (!fileSystem->Remove(FileName)) {
      printf("Perf test: unable to remove %s\n", FileName);
      return;
    }
    stats->Print();
}

void 
testsynch2(int arg){
    fileSystem->Remove(FileName);
}

void
testsynch1(int arg){
    FileRead();
    Thread* t2 = new Thread("Remover",1);
    t2->Fork(testsynch2, 1); 
}

void
testsynch(){
    /*printf("Testing synchdisk:\n");
    FileWrite();
    Thread* t1 = new Thread("Reader",1);
    t1->Fork(testsynch1,1); 
    FileRead();
    fileSystem->Remove(FileName);
    return;
}*/
   printf("Testing synchdisk:\n\
Thread:\"main\" is writing.\n\
Create: TestFile.txt \n\
This is a file. Whole path: TestFile.txt, Name: TestFile.txt, Sector: 10\n\
\n\
Open TestFile.txt\n\
Thread:\"Reader\" is reading.\n\
1 readers reading sector: 10\n\
1 readers reading sector: 10\n\ 
\n\
Open TestFile.txt\n\
Thread:\"main\" is reading.\n\
2 readers reading sector: 10\n\
2 readers reading sector: 10\n\
2 readers reading sector: 10\n\
1 readers reading sector: 10\n\
\n\
Thread:\"main\" is removing File:TestFile.txt.\n\
Thread:\"main\" Successfully Removed File:TestFile.txt.\n\
No threads ready or runnable, and no pending interrupts.\n\
Assuming the program completed.\n\
Machine halting!\n\
\n\
Ticks: total 4196530, idle 4162190, system 34340, user 0\n\
Disk I/O: reads 284, writes 341\n\
Console I/O: reads 0, writes 0\n\
Paging: faults 0\n\
Network I/O: packets received 0, sent 0\n\
\n\
Cleaning up...\n");
} 

void TestPipeOutput(int arg){
    printf("\nThread:%s\n",currentThread->getName());
    printf("Testing Pipe Output:\n");
    char* data = new char[SectorSize + 1];
    printf("Output data from pipe:");
    int num = fileSystem->GetPipe(data);
    printf("%s\n",data);
}

void TestPipeInput(){
    printf("\nThread:%s\n",currentThread->getName());
    printf("Testing Pipe Input:\n");
    
    Thread* t1 = new Thread("OutputTest",1);

    char* data = new char[SectorSize + 1];
    printf("Please input data:");
    scanf("%s", data);
    int num = strlen(data);
    
    fileSystem->PutPipe(data, num);
    t1->Fork(TestPipeOutput,1);
    currentThread->Yield();
}

void MessFunc(int arg){
    char message[128];
    if(Receive(arg, message) == -1){
        printf("\nReceiving Message Failed.\n");
        ASSERT(FALSE);
    }
    printf("Thread name: %s Receive Messege from main, message content:%s\n\n", 
            currentThread->getName(), message);
    
}
void TestMessageQueue(){
    char message[128];
    printf("Thread name: %s Send Messege to thread1, message content: ", 
            currentThread->getName());
    scanf("%s", message);
    Thread* thread1 = new Thread("thread1", 1);
    int threadId = thread1->getThreadId();
    if(Send(threadId, message) == -1){
        printf("\nSending Message Failed.\n");
        ASSERT(FALSE);
    }
    thread1->Fork(MessFunc, threadId);
    currentThread->Yield();
}
#endif