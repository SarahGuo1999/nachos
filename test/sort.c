/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"
#define Asize 100  // orginal:1024

int A[Asize];	/* size of physical memory; with code, we'll run out of space!*/

int
main()
{
    int i, j, tmp,k;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < Asize; i++)	{
        A[i] = Asize - i;
    }	
    

    /* then sort! */
    for (i = 0; i < Asize - 1; i++)
        for (j = i; j < (Asize - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }

    for(k = 0;k < Asize;++k)
        A[k] = 2;
    Halt();
    //Exit(A[0]);		/* and then we're done -- should be 0! */
}
