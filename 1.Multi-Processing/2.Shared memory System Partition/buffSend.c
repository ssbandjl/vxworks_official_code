/* buffSend.c - simple buffer exchange protocol send side */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01b,17sept97,ram tested ok
01a,27jan94,jl	 cleanup of code in programmers manual	
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
* buffSend - write to shared semaphore protected buffer
*
*/

STATUS buffSend (void)

	{

	SHARED_BUFF * pSharedBuff;

	/* grab shared system memory */

	pSharedBuff = (SHARED_BUFF *) smMemMalloc (sizeof (SHARED_BUFF));

	/*
	 * Initialize shared buffer structure before adding to database. The
	 * protection semaphore is initially unavailable and receiver will block.
	 */

	if ((pSharedBuff->semSmId = semBSmCreate (SEM_Q_FIFO, SEM_EMPTY)) == NULL)
		return (ERROR);

	/*
	 * Convert address of shared buffer to a global address and add to
	 * database.
	 */

	if (smNameAdd (BUFF_NAME, (void *) smObjLocalToGlobal (pSharedBuff), 
                    T_SM_BLOCK) == ERROR)
		return (ERROR);

	/* put data into shared buffer */

	sprintf (pSharedBuff->buff,"Hello from sender\n");

	/* allow receiver to read data by giving protection semaphore */

	if (semGive (pSharedBuff->semSmId) != OK)
		return (ERROR);

	return (OK);
	}

