/* buffReceive.c - simple buffer exchange protocol receive side */ 

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
o1b,17sept97,ram tested ok
01a,27jan94,jl	 cleanup of code in programmer's manual	
*/

#include "vxWorks.h"
#include "semLib.h"
#include "semSmLib.h"
#include "smNameLib.h"
#include "smObjLib.h"
#include "stdio.h"
#include "buffProtocol.h"

/************************************************************************
*
* buffReceive - receive shared semaphore protected buffer
*
*/

STATUS buffReceive (void)

	{

	SHARED_BUFF * pSharedBuff;
	int           objType;

	/* get shared buffer address from name database */

	if (smNameFind (BUFF_NAME, (void **) &pSharedBuff, 
                     &objType, WAIT_FOREVER) == ERROR)
		return (ERROR);

	/* convert global address of buff to its local value */

	pSharedBuff = (SHARED_BUFF *) smObjGlobalToLocal (pSharedBuff);

	/* take shared semaphore before reading the data buffer */

	if (semTake (pSharedBuff->semSmId,WAIT_FOREVER) != OK)
		return (ERROR);

	/* read data buffer and print it */

	printf ("Receiver reading from shared memory: %s\n", pSharedBuff->buff);

	/* give back the data buffer semaphore */

	if (semGive (pSharedBuff->semSmId) != OK)
		return (ERROR);

	return (OK);
	}

