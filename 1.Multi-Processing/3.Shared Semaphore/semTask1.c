/* semTask1.c - shared semaphore example */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01b,17seot97,ram  need to add INCLUDE_SM_OBJ in configAll.h
		  tested OK
01,27jan94,jl	  cleanup of code in programmers manual	
*/

#include <vxWorks.h>
#include "semLib.h"
#include "semSmLib.h"
#include "smNameLib.h"
#include "stdio.h"
#include "taskLib.h"
#include "semExample.h"
#include "sysLib.h"

/************************************************************************
*
* semTask1 - shared semaphore user 
*
*/

STATUS semTask1 (void)
	{
	SEM_ID semSmId;

	/* create shared semaphore */

	if ((semSmId = semBSmCreate (SEM_Q_FIFO, SEM_FULL)) == NULL)
		return (ERROR);

	/* add object to name database */

	if (smNameAdd (SEM_NAME, semSmId, T_SM_SEM_B) == ERROR)
		return (ERROR);

	/* grab shared semaphore and hold it for awhile */

	semTake (semSmId, WAIT_FOREVER);

	/* normally do something useful */

	printf ("Task1 has the shared semaphore\n");
	taskDelay (sysClkRateGet () * 5);
	printf ("Task1 is releasing the shared semaphore\n");

	/* release shared semaphore */
	
	semGive (semSmId);

	return (OK);
	}

