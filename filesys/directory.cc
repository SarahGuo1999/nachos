// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include "filesys.h"
#include "system.h"
#include "thread.h"

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1


#define NumDirEntries 10
//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++){
	    table[i].inUse = FALSE;
        table[i].IsDirectory = FALSE;
    }
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{  
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);

}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------
 
void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, 3))///strlen(table[i].name)))
	    return i;
    
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    //printf("hhhh   %d\n",strlen(name));
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}


int 
Directory::GetRootDirSector(char* name){
    Directory *directory = new Directory(NumDirEntries);
    OpenFile* DirFile = new OpenFile(DirectorySector);
    directory->FetchFrom(DirFile);
    char CurDirName[FileNameMaxLen + 1];
    int idx = 0;
    int sector = DirectorySector;
    // find sector of the file / directory
    for(int i = 0;i < strlen(name);++i){
        if(name[i] == '/' ){
            strncpy(CurDirName, name + idx, i-idx);
            CurDirName[i-idx] = '\0';
            printf("Finding Directory: %s\n",CurDirName);
            sector = directory->Find(CurDirName);
            DirFile = new OpenFile(sector);
            directory = new Directory(NumDirEntries);
            directory->FetchFrom(DirFile);
            idx = i + 1;
        }
    }
    return sector;
}


void
Directory::GetFileName(char* name, char* FileName){
    //printf("name:%s|filename:%s\n",name,FileName);
    bool HasDir = FALSE;
    for(int i = strlen(name) - 1; i >= 0;--i){
        if(name[i] == '/'){
            HasDir = TRUE;
            int j = i + 1;
            for(; j < strlen(name);++j)
                FileName[j - i - 1] = name[j];
            FileName[j - i - 1] = '\0';
            break;
        }
    }
    if(!HasDir)
        strncpy(FileName, name, strlen(name)+1);
}


//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector, bool IsDir)
{ 
    char FileName[FileNameMaxLen + 1];
    
    GetFileName(name, FileName);
    if(IsDir){
        strncpy(FileName,name, strlen(name));
        FileName[strlen(name)] = '\0';
    }   
    if(!IsDir)
        printf("This is a file.  Whole path: %s,  Name: %s,  Sector: %d\n"\
                ,name, FileName, newSector);
    else
        printf("This is a directory.  Whole path: %s,  Name: %s,  Sector: %d \n"\
                ,name, FileName, newSector);
    if(FindIndex(FileName) != -1)      return FALSE;

    for (int i = 0; i < tableSize; i++){
        if (!table[i].inUse) {
            strncpy(table[i].name, FileName,strlen(FileName));
            strncpy(table[i].PathName, name, strlen(name));
            table[i].IsDirectory = IsDir;
            table[i].sector = newSector;
            table[i].inUse = TRUE;
            return TRUE;
	    }
    }
    return FALSE;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    return TRUE;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
    //printf("directory list\n");
   for (int i = 0; i < tableSize; i++)
	if (table[i].inUse)
	    printf("%s\n", table[i].name);
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    printf("Directory Preview:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
        char FileName[strlen(table[i].name)];
        GetFileName(table[i].name,FileName);

        printf("Whole Path:%s,  Name: %s, Sector: %d\n",\
                table[i].name, FileName, table[i].sector);
	}
    printf("\n");

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
         printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;
}
