// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"
#include "time.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
    
    if(numSectors > DirectIndexNum){  // need indirect index
        printf("\n*********! Use Indirect Index !*********\n\n");

        int SecIndex[SecNumDirect * IndirectIndexNum];     // array of second page table
        for(int i = 0;i < DirectIndexNum;++i)              // find direct index
            dataSectors[i] = freeMap->Find();
        for(int i = DirectIndexNum;i < NumDirect;++i)      // find indirect index
            dataSectors[i] = freeMap->Find();
        for(int i = 0;i < numSectors - DirectIndexNum;++i)  // assign the rest sector pages
            SecIndex[i] = freeMap->Find();
        // Write the address of SecIndex to the last page in first page table.
        synchDisk->WriteSector(dataSectors[DirectIndexNum], (char*)(SecIndex)); 
    }
    else{
        for (int i = 0; i < numSectors; i++)
	        dataSectors[i] = freeMap->Find();
    }
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    if(numSectors > DirectIndexNum){  // need undirect index
        char* SecIndex = new char[SectorSize * IndirectIndexNum];
        synchDisk->ReadSector(dataSectors[DirectIndexNum], (char*)(SecIndex ));
        
        int end = numSectors - DirectIndexNum;  // calculate the number of indirect page
        for(int i = 0;i < end;++i){
            ASSERT(freeMap->Test((int) SecIndex[i * 4]));  // ought to be marked!
            freeMap->Clear((int)SecIndex[i * 4]);       // clear sector according to second page index 
        }

        for (int i = 0; i < NumDirect; i++) {
            ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }
    }
    else{
        for (int i = 0; i < numSectors; i++) {
            //ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    // printf("FileHeader::FetchFrom  %s\n",(char*)this);
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    //printf("FileHeader::WriteBack %d %s\n",sector,(char*)this);
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    if(offset < SectorSize * DirectIndexNum)
        return(dataSectors[offset / SectorSize]);
    else{
        char* SecIndex = new char[SectorSize * IndirectIndexNum];
        synchDisk->ReadSector(dataSectors[DirectIndexNum], (char*)(SecIndex));
        
        int pos = (offset - SectorSize * DirectIndexNum) / SectorSize;
        return (int)(SecIndex[pos * 4]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

void 
FileHeader::SetFileLength(int numbytes)
{
    numBytes = numbytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    
    if(numSectors > DirectIndexNum){  // need undirect index
        printf("IndirectIndex : %d\n",dataSectors[DirectIndexNum]);
        for (i = 0; i < DirectIndexNum; ++i)
	        printf("%d ", dataSectors[i]);
        char* SecIndex = new char[SectorSize * IndirectIndexNum];
        synchDisk->ReadSector(dataSectors[DirectIndexNum], (char*)(SecIndex));
        for (i = k = 0; i < numSectors - DirectIndexNum;++i,k += 4)
            printf("%d ",(int)(SecIndex[k]));
    }
    else{
        for (i = 0; i < numSectors; i++)
	        printf("%d ", dataSectors[i]);
    }

    printf("\nFile contents:\n");

    if(numSectors > DirectIndexNum){
        for (i = k = 0; i < DirectIndexNum; i++) {
            printf("Sector : %d\n", dataSectors[i]);
	        synchDisk->ReadSector(dataSectors[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		        printf("%c", data[j]);
                else
		        printf("\\%x", (unsigned char)data[j]);
	        }
            printf("\n"); 
        }
        char* SecIndex = new char[SectorSize * IndirectIndexNum];
        synchDisk->ReadSector(dataSectors[DirectIndexNum], (SecIndex));

        for(i = 0; i < numSectors - DirectIndexNum;++i){
            printf("Sector : %d\n", (int)(SecIndex[i * 4]));
            synchDisk->ReadSector((int)(SecIndex[4 * i]), data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                printf("%c", data[j]);
                    else
                printf("\\%x", (unsigned char)data[j]);
	        }
            printf("\n"); 
        }
    }
    else{
        for (i = k = 0; i < numSectors; i++) {
            synchDisk->ReadSector(dataSectors[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                printf("%c", data[j]);
                    else
                printf("\\%x", (unsigned char)data[j]);
            }
            printf("\n"); 
        }
    }
    printf("Create Time: %s\n", CreateTime);
    printf("Last Visit Time: %s\n", VisitTime);
    printf("Last Modified Time: %s\n", ModifiedTime);
    printf("File Type: %s\n", FileType);
    delete [] data;
}


//----------------------------------------------------------------------
// FileHeader::SetCreateTime & SetVisitTime & SetModifiedTime
// 	Set create time and last visit time and last modified time of the file.
//----------------------------------------------------------------------

void
FileHeader::SetCreateTime(){
    time_t t;
    time(&t);
    strncpy(CreateTime, asctime(gmtime(&t)), TimeLength);
    CreateTime[TimeLength - 1] = '\0';
    // printf("Set Create Time to %s\n", CreateTime);
}

void
FileHeader::SetVisitTime(){
    time_t t;
    time(&t);
    strncpy(VisitTime, asctime(gmtime(&t)), TimeLength);
    VisitTime[TimeLength - 1] = '\0';
    //printf("Set Last Visit Time to %s\n", VisitTime);
}

void
FileHeader::SetModifiedTime(){
    time_t t;
    time(&t);
    strncpy(ModifiedTime, asctime(gmtime(&t)), TimeLength);
    ModifiedTime[TimeLength - 1] = '\0';
    //printf("Set Last Modified Time to %s\n", ModifiedTime);
}

void 
FileHeader::SetFileType(char* filetype){
    int i = 0;
    for(;filetype[i]!='\0';++i)
        FileType[i] = filetype[i];
    FileType[i] = '\0';
    //printf("Set File Class: %s\n", FileType);
}

void 
FileHeader::SetFileSector(int sector){
    FileSector = sector;
    //printf("Set File Sector: %d\n", sector);
}