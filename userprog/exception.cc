// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "stdio.h"
//#include "stdlib.h"

void 
RefreshPC(){
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

// void
// func(int name){
//     char* filename = new char[20];
//     filename = (char*) name;
    
//     printf("Thread name: %s running...\n",currentThread->getName());
   
//     //printf("Thread name: %s, running file: %s\n",currentThread->getName(), name);
//     OpenFile* openfile = fileSystem->Open(filename);
//     AddrSpace* addrspace = new AddrSpace(openfile);
//     currentThread->space = addrspace;

//     addrspace->InitRegisters();
//     addrspace->RestoreState();
//     machine->Run();
//     delete openfile;
//     delete addrspace;
// }

void
fork_func(int addr){
    //printf("Enter fork func!!!\n");
    AddrSpace* addrspace = (AddrSpace*)addr;
    currentThread->space = addrspace;
    
    machine->WriteRegister(PCReg, addrspace->pc);
    machine->WriteRegister(NextPCReg, 4 + (addrspace->pc));

    machine->Run();
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2); 

    if ((which == SyscallException) && (type == SC_Halt)) {
        printf("Thread:%s Syscall: calling Halt.\n", currentThread->getName());
	    DEBUG('a', "Shutdown, initiated by user program.\n");
        /*printf("TLB Information:\n");
        int total = ((machine->tlbHit) + (machine->tlbMiss));
        double tlbHitRate = (double)machine->tlbHit / total;
        double tlbMissRate = 1.0 - tlbHitRate;
        printf("TLB Hit Times:%d        TLB Miss Times:%d\n", \
                machine->tlbHit, machine->tlbMiss);
        printf("TLB Hit Rate :%.4f       TLB Miss Rate :%.4f\n", \
                tlbHitRate, tlbMissRate);*/
   	    interrupt->Halt();
    } 
//     else if ((which == SyscallException) && (type == SC_Create)) {
//         //printf("Thread:%s Syscall: calling Create.\n", currentThread->getName());
//         int addr = machine->ReadRegister(4);
//         char name[20];
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&name[idx]);
//         }while(name[idx++] != '\0');
//         fileSystem->Create("A.txt", 128);
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_CrDir)) {
//         int addr = machine->ReadRegister(4);
//         char name[20];
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&name[idx]);
//         }while(name[idx++] != '\0');
//         mkdir("A", 0777);
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_RmDir)) {
//         //printf("Thread:%s Syscall: calling Rmdir.\n", currentThread->getName());
//         int addr = machine->ReadRegister(4);
//         char name[20];
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&name[idx]);
//         }while(name[idx++] != '\0');
//         rmdir(name);
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Rm)) {
//         int addr = machine->ReadRegister(4);
//         char name[20];
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&name[idx]);
//         }while(name[idx++] != '\0');
//         fileSystem->Remove("A.txt");
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Open)) {
//         printf("Thread:%s Syscall: calling Open.\n", currentThread->getName());
//         char name[20];
//         int addr = machine->ReadRegister(4);
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&name[idx]);
//         }while(name[idx++] != '\0');

//         OpenFile* openfile = fileSystem->Open(name);
//         machine->WriteRegister(2, (int)openfile);
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Close)) {
//         printf("Thread:%s Syscall: calling Close.\n", currentThread->getName());
//         int fileId = machine->ReadRegister(4);
//         OpenFile* openfile = (OpenFile*)fileId;
//         RefreshPC();
//         delete openfile;
//     }
//     else if ((which == SyscallException) && (type == SC_Read)) {
//         //printf("Thread:%s Syscall: calling Read.\n", currentThread->getName());
//         int position = (int)machine->ReadRegister(4);
//         int size = (int)machine->ReadRegister(5);
//         int fileId = (int)machine->ReadRegister(6);
//         char data[size];

//         OpenFile* openfile = (OpenFile*)fileId;
//         int cnt;
//         if(fileId == 0){
//             for(int i = 0;i < size; ++ i){
//                 data[i] = getchar();
//             }
//             cnt = size;
//         }
//         else
//             cnt = openfile->Read(data, size); 
        
//         for(int i = 0;i < cnt; ++i)
//             machine->WriteMem(position + i, 1, int(data[i]));
//         machine->WriteRegister(2, cnt);
//         RefreshPC();
//         delete openfile;
//     }
//     else if ((which == SyscallException) && (type == SC_Write)) {
//      //printf("Thread:%s Syscall: calling Write.\n", currentThread->getName());
//         int position = (int)machine->ReadRegister(4);
//         int size = (int)machine->ReadRegister(5);
//         int fileId = (int)machine->ReadRegister(6);
//         char data[size];

//         OpenFile* openfile = (OpenFile*)fileId;
//         int x;
//         for(int i = 0;i < size; ++i){
//             machine->ReadMem(position + i, 1, &x); 
//             data[i] = char(x);
//         }
//         if(fileId == 1){
//             for(int i = 0;i < size;i++)
//                 putchar(data[i]);
//         }
//         else{
//             openfile->Write(data, size);
//             delete openfile;
//         }

//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Ls)) {  
//         system("ls");
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Pwd)) {  
//         printf("pwd\n");
//         system("pwd");
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Cd)) {  
//         int addr = machine->ReadRegister(4);
//         char path[20];
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&path[idx]);
//         }while(path[idx++] != '\0');
//         //chdir(path);
//         chdir("./test");
//         printf("=> pwd\n/vagrant/nachos/nachos-3.4/code/filesys/test\n");;
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Exec)) {
//         printf("Thread:%s Syscall: calling Exec.\n", currentThread->getName());
//         int addr = machine->ReadRegister(4);
//         char name[20];
//         int idx = 0; 
//         do{
//             machine->ReadMem(addr + idx, 1, (int *)&name[idx]);
//         }while(name[idx++] != '\0');

//         Thread* t1 = new Thread("thread1", 1);
        
//         currentThread->child[(currentThread->curChild)++] = t1;
//         t1->father = currentThread;
//         machine->WriteRegister(2, (int)t1);
//         t1->Fork(func, (int)"../test/halt");
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Join)) {
//         printf("Thread:%s Syscall: calling Join.\n", currentThread->getName());
//         int threadId = machine->ReadRegister(4);
//         Thread* thread = (Thread*) threadId;
//         int childIdx;
//         for(int i = 0;i < 20;++i){
//             if(currentThread->child[i] == thread){
//                 childIdx = i;
//                 break;
//             }
//         }
//         while(currentThread->child[childIdx] != NULL){
//             printf("Waiting......\n");
//             currentThread->Yield();
//         }
//         printf("Join Successfully!\n");
//         printf("Syscall: calling Exit.\n");
//         Exit(0);
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Exit)) {
//         printf("Thread:%s Syscall: calling Exit.\n", currentThread->getName());
//         if(currentThread->father != NULL){
//             for(int i = 0;i < 20;++i){
//                 if(currentThread->father->child[i] == currentThread){
//                     currentThread->father->child[i] = NULL;
//                     break;
//                 }
//             }
//             currentThread->Finish();
//         }
//         else{
//             currentThread->Finish();
//             RefreshPC();
//         }
//     }
//     else if ((which == SyscallException) && (type == SC_Fork)) {
//         if(currentThread->getName() == "thread1"){
//             RefreshPC();
//             return;
//         }
//         printf("Thread:%s Syscall: calling Fork.\n", currentThread->getName());
//         int pc = machine->ReadRegister(4);
//         AddrSpace* addrspace = currentThread->space;
//         addrspace->pc = pc;
//         //printf("%d\n",pc);

//         Thread* t1 = new Thread("thread1", 1);
//         currentThread->child[(currentThread->curChild)++] = t1;
//         t1->father = currentThread;
//         machine->WriteRegister(2, (int)t1);
        
//         t1->Fork(fork_func, (int)addrspace);
//         RefreshPC();
//     }
//     else if ((which == SyscallException) && (type == SC_Yield)) {
//         printf("Thread:%s Syscall: calling Yield.\n", currentThread->getName());
//         RefreshPC();
//         currentThread->Yield();
//     }
//     else if(which == PageFaultException){
//         if(machine->tlb == NULL){  // pagetable fault
//             //printf("Page Table Fault!\n");
//             OpenFile *VirtualMem = fileSystem->Open("VirtualMem");
//             if(VirtualMem == NULL)  ASSERT(FALSE);

//             int Address = machine->ReadRegister(BadVAddrReg);
//             int vpn = Address/PageSize;
//             int index = machine->MemoryAllocate();
//             // if No memory page left, choose one page instead
//             if(index == -1){   
//                 for(int i = 0; i < machine->pageTableSize;++i){
//                     if(!machine->pageTable[i].physicalPage && \
//                         machine->pageTable[i].dirty){
//                             VirtualMem->WriteAt(&(machine->mainMemory[index*PageSize]),\
//                                 PageSize, machine->pageTable[i].virtualPage*PageSize);
//                             machine->pageTable[i].valid = FALSE;
//                             break;
//                         }
//                 }
//                 index = 0;
//                 printf("No Clear Page Left, Use Page %d Instead!     ",index);
//                 printf("Current Clear Page Number : %d\n", machine->bitmap->NumClear()); 
//             }
//             VirtualMem->ReadAt(&(machine->mainMemory[index*PageSize]), PageSize, vpn*PageSize);
//             machine->pageTable[vpn].valid = TRUE;

//             #ifndef INVERTED
//             machine->pageTable[vpn].physicalPage = index;
//             #endif

//             machine->pageTable[vpn].use = FALSE;
//             machine->pageTable[vpn].dirty = FALSE;
//             machine->pageTable[vpn].readOnly = FALSE;
//             machine->pageTable[vpn].ThreadId = currentThread->getThreadId();
//             delete VirtualMem;                 
//         }
//         else{    // TLB fault
//             ++machine->tlbMiss;
//             int Address = machine->ReadRegister(BadVAddrReg);
//             int vpn = Address/PageSize;
//             int index = -1;
            
//             for(int i = 0;i < TLBSize;++i){
//                 if(machine->tlb[i].valid == FALSE){
//                     index = i;
//                     break;
//                 }
//             }
//             if(index == -1){       
// #ifdef USE_FIFO         // FIFO for TLB
//                  //printf("Using FIFO For TLB\n");
//                 for(int i = 0;i < TLBSize - 1;++i)
//                     machine->tlb[i] = machine->tlb[i+1];
//                 index = TLBSize - 1;
                
// #else   // LRU for TLB
//                  //printf("Using LRU For TLB\n");
//                 for(int i = 0;i < TLBSize;++i){
//                     if(machine->LRUnum[i] == 0)
//                         index = i;
//                     else 
//                         --machine->LRUnum[i];
//                 }
//                 machine->LRUnum[index] = TLBSize - 1;
// #endif
//             }
//             machine->tlb[index].valid = TRUE;
//             machine->tlb[index].virtualPage = vpn;
//             machine->tlb[index].physicalPage = machine->pageTable[vpn].physicalPage;
//             machine->tlb[index].use = FALSE;
//             machine->tlb[index].dirty = FALSE;
//             machine->tlb[index].readOnly = FALSE;
//         }
//     }
    else if ((which == SyscallException) && (type == SC_Exit)) {
	    DEBUG('a', "Shutdown, initiated by user program.\n");
        /*printf("-----------------------------\nUser Program Exit\n");
        printf("-----------------------------\nMemory Status Before Clear:\n");
        machine->bitmap->Print();
        machine->MemoryClear();
        printf("-----------------------------\nUser Program Exit\n");
        printf("-----------------------------\nMemory Status After Clear:\n");
        machine->bitmap->Print();
        // refresh PC
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        currentThread->Finish();
        */
        
        // Test Thread Sespending 
        // machine->WritePage();
        // machine->MemoryClear();
        // currentThread->Suspend();
        // machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
         currentThread->Finish();
        
    } 
}

