/* semTask2.c - shared semaphore example */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01b,17sept97,ram  need to add INCLUDE_SM_OBJ in configAll.h
		  tested OK
01a,27jan94,jl	  cleanup of code in programmers manual	
*/

#include "vxWorks.h"
#include "semLib.h"
#include "semSmLib.h"
#include "smNameLib.h"
#include "stdio.h"
#include "semExample.h"

/************************************************************************
*
* semTask2 - shared semaphore user 
*
*/

STATUS semTask2 (void)
	{
	SEM_ID semSmId;
	int    objType;

	/* find object in name database */

	if (smNameFind (SEM_NAME, (void **) &semSmId, &objType, WAIT_FOREVER)
		== ERROR)
		return (ERROR);

	/* take the shared semaphore */

	printf ("semTask2 is now going to take the shared semaphore\n");
	semTake (semSmId, WAIT_FOREVER);

	/* normally do something useful */

	printf ("Task2 got the shared semaphore!!\n");

	/* release shared semaphore */

	semGive (semSmId);

	printf ("Task2 has released the shared semaphore\n");

	return (OK);
	}
