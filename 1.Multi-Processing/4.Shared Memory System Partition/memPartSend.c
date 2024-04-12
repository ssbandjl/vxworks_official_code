
/* memPartSend.c - shared memory partition example send side */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
o1b,17sept97,ram removed the smObjGlobalToLocal cast applied to pMem. 
01a,1feb94,jl	 cleanup of code in programmers manual
*/

#include "vxWorks.h"
#include "memLib.h"
#include "semLib.h"
#include "semSmLib.h"
#include "smNameLib.h"
#include "smObjLib.h"
#include "smMemLib.h"
#include "stdio.h"
#include "memPartExample.h"


/*********************************************************************
*
* memPartSend - send shared memory partition buffer
* 
*/


STATUS memPartSend (void)

	{
	char *         pMem;
	PART_ID        smMemPartId;
	SHARED_BUFF *  pSharedBuff;

	/* allocate shared system memory to use for partition */

	pMem = smMemMalloc (CHUNK_SIZE);


	/* Create user defined partition using the previously allocated
	 * block of memory.
	 * WARNING: memPartSmCreate uses the global address of a memory
	 * pool as first parameter.
	 */

	if ((smMemPartId = memPartSmCreate (pMem, CHUNK_SIZE)) == NULL)
		return (ERROR);

	/* allocate memory from partition */

	pSharedBuff = (SHARED_BUFF *) memPartAlloc ( smMemPartId, 
				sizeof (SHARED_BUFF));
	if (pSharedBuff == 0)
		return (ERROR);

	/* initialize structure before adding to database */

	if ((pSharedBuff->semSmId = semBSmCreate (SEM_Q_FIFO, SEM_EMPTY)) 
          == NULL)
		return (ERROR);

	/* enter shared partition ID in name database */

	if (smNameAdd (MEM_PART_NAME, (void *) smMemPartId, T_SM_PART_ID) == ERROR)
		return (ERROR);

	/* convert shared buffer address to a global address and add to database */

	if (smNameAdd (PART_BUFF_NAME, (void *) smObjLocalToGlobal(pSharedBuff), 
                   T_SM_BLOCK) == ERROR)
		return (ERROR);

	/* send data using shared buffer */

	sprintf (pSharedBuff->buff,"Hello from sender\n");

	if (semGive (pSharedBuff->semSmId) != OK)
		return (ERROR);

	return (OK);
	}

