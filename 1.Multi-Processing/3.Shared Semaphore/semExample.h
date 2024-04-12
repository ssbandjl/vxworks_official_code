/*******

Shared Semaphore Code Example

The following code example depicts two tasks executing on different CPUs and using 
shared semaphores.  The routine semTask1( ) creates the shared semaphore, initializing 
the state to full.  It adds the semaphore to the name database (to enable the task on the 
other CPU to access it), takes the semaphore, does some processing, and gives the sema-
phore.  The task semTask2( ) gets the semaphore ID from the database, takes the sema-
phore, does some processing, and gives the semaphore.

   o	The common header file is semExample.h:
****************/

/* semExample.h - shared semaphore example header file */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.    
01a,???  written.
*/

#define SEM_NAME "mySmSemaphore"
