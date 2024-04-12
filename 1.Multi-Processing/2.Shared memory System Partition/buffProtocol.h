/*******************
 Shared Memory System Partition Code Example

The following code example uses memory from the shared memory system partition  
to share data between tasks on different CPUs.  The first member of the data 
structure is a shared semaphore that is used for mutual exclusion.  The send 
task creates and initializes the structure.  The receive task simply accesses 
the data and displays it. 
**********************/

/* buffProtocol.h - simple buffer exchange protocol header file */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.    
01a,???  written.
*/

#define BUFFER_SIZE   200           /* shared data buffer size */ 
#define BUFF_NAME     "myMemory"    /* name of data buffer in database */

typedef struct shared_buff 

	{
	SEM_ID semSmId;
	char   buff [BUFFER_SIZE];
	} SHARED_BUFF;
