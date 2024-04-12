/*********************
User-Created Partition Code Example

The following example is similar to the previous example using the shared memory sys-
tem partition.  This example creates a user-defined partition and stores the shared data 
in this new partition.  A shared semaphore is used to protect the data.

   o	The common header file memPartExample.h follows:
**********************/

/* memPartExample.h - shared memory partition example header file */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.    
01a,???  written.
*/

#define CHUNK_SIZE      (2400)

#define MEM_PART_NAME   "myMemPart"

#define PART_BUFF_NAME  "myBuff"

#define BUFFER_SIZE     (40)



typedef struct shared_buff

	{

	SEM_ID semSmId;

	char   buff [BUFFER_SIZE];

	} SHARED_BUFF;
