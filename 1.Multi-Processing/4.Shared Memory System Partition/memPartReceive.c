/* memPartReceive.c - shared memory partition example receive side */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01b,17sept97,ram need to include INCLUDE_SM_OBJ in configAll.h 
		 added include file smNameLib.h, smObjLib.h
01a,1feb94,jl	 cleanup of code in programmers guide
*/

#include "vxWorks.h"
#include "memLib.h"
#include "stdio.h"
#include "semLib.h"
#include "semSmLib.h"
#include "stdio.h"
#include "memPartExample.h"
#include "smNameLib.h"
#include "smObjLib.h"

/*********************************************************************
*
* memPartReceive - receive shared memory partition buffer 
*
* execute on CPU 1 - use a shared semaphore to protect shared memory
*
*/



STATUS memPartReceive (void)
	{
	SHARED_BUFF * pBuff;
	int           objType;

	/* get shared buffer address from name database */

	if (smNameFind (PART_BUFF_NAME, (void **) &pBuff, &objType, 
                    WAIT_FOREVER) == ERROR)
		return (ERROR);

	/* convert global address of buffer to its local value */

	pBuff = (SHARED_BUFF *) smObjGlobalToLocal (pBuff);

	/* Grab shared semaphore before using the shared memory */

	semTake (pBuff->semSmId,WAIT_FOREVER);

	printf ("Receiver reading from shared memory: %s\n", pBuff->buff);

	semGive (pBuff->semSmId);

	return (OK);
	}

