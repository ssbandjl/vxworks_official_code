/* privateCode.c - uses VM contexts to make data private to a code segment */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01d,06nov97,rv   added copyright.
01c,21oct97,mm   cast arg1 of bzero , cast arg 1 and 2 of bcopy
01b,22sep97,ram  tested OK
01a,27jan94,jl	 written
*/

/* includes */

#include "vxWorks.h"
#include "vmLib.h"
#include "semLib.h"
#include "privateCode.h"
#include "string.h"

/* globals */
MY_DATA * pData;
SEM_ID dataSemId;
int pageSize;

/***************************************************************************
 *
 * initData - allocate memory and make it non-writable
 *
 * This routine initializes data and should be called only once.
 *
 */


STATUS initData ( void )
    {
    pageSize = vmPageSizeGet ();

    /* create semaphore to protect data */

    dataSemId = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);

    /* allocate memory = to a page */

    pData = (MY_DATA *) valloc (pageSize);

    /* initialize data and make it read-only */

    bzero ((char *)pData, pageSize);
    if (vmStateSet (NULL, pData, pageSize, VM_STATE_MASK_WRITABLE,
	    VM_STATE_WRITABLE_NOT) == ERROR)
		{
		semGive (dataSemId);
		return (ERROR);
		}
    /* release semaphore */
    semGive(dataSemId);
    return (OK);
    }

/***************************************************************************
 *
 * dataModify - modify data
 *
 * To modify data, tasks must call this routine, passing a pointer to
 * the new data.
 *
 * To test from the shell use:
 * -> initData
 * -> sp dataModify
 * -> d pData
 * -> bfill (pData, 1024, 'X')
 *
 */


STATUS dataModify (
    MY_DATA * pNewData
    )
    {
    /* take semaphore for exclusive access to data */
    semTake (dataSemId, WAIT_FOREVER);
    /* make memory writeable */
    if (vmStateSet (NULL, pData, pageSize, VM_STATE_MASK_WRITABLE,
	    VM_STATE_WRITABLE) == ERROR)
		{
		semGive (dataSemId);
		return (ERROR);
		}
    /* update data */
    bcopy ((const char *) pNewData, (char *)pData, sizeof(MY_DATA));
    /* make memory not writable */
    if (vmStateSet (NULL, pData, pageSize, VM_STATE_MASK_WRITABLE,
	    VM_STATE_WRITABLE_NOT) == ERROR)
		{
		semGive (dataSemId);
		return (ERROR);
		}
    semGive (dataSemId);
    return (OK);
    }
