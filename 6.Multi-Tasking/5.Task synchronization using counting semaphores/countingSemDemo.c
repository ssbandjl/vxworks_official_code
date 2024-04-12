/* countingSemDemo.c - Demonstrates task synchronization using counting 
 *                     semaphores.
 */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm  added copyright.
01b,16sep97,ram included files stdio.h, taskLib.h, usrLib.h, sysLib.h
		 Used the semShow() function to display info about
		 the semaphore inplace of the show() function.
01a,27mar94,ms   cleaned up for VxDemo
*/

/*
 * DESCRIPTION
 * Counting semaphore example.  Using binary semaphores for task 
 * synchronization may, if the events can occur rapidly enough, cause 
 * a loss of data, i.e. an event can be missed if the events can occur
 * faster then a task can process them.  Using counting semaphores may
 * solve this problem. This program demonstrates task synchronization using
 * counting semaphores. User can also select to use a binary semaphore instead 
 * of a counting semaphore in this demonstration, for comparision between the  
 * two semaphores.
 *
 * RETURNS: OK or ERROR
 *
 * EXAMPLE
 *
 *  -> sp countingSemDemo, typeOfSemaphore
 *
 *  where typeOfSemaphore is the value of the type of semaphore to be used
 *  for task synchronization in this demonstration. For using counting 
 *  semaphores use a value 'c' or 'C' for typeOfSemaphore parameter and for 
 *  using binary semaphores use a value 'b' or 'B' for typeOfSemaphore 
 *  parameter. 
 *
 *  example:
 *  -> sp countingSemDemo, 'c'
 *  -> sp countingSemDemo, 'b'
 *
 */

/* include files */

#include "vxWorks.h"
#include "wdLib.h"
#include "stdio.h"
#include "semLib.h"
#include "taskLib.h"
#include "usrLib.h"
#include "sysLib.h"

/* defines */

#define TASK_PRIORITY				101
#define TASK_STACK_SIZE				5000
#define TIME_BETWEEN_INTERRUPTS		        1   /* 1 tick */
#define TASK_WORK_TIME				2   /* 2 ticks */
#define NUM_OF_GIVES				3   

/* globals */

LOCAL SEM_ID semId = NULL;		/* counting or binary semaphore ID */
LOCAL WDOG_ID wdId = NULL;              /* watchdog ID */
LOCAL int syncTaskTid = 0;              /* tid of syncTask */
LOCAL int numToGive = NUM_OF_GIVES;     /* Number of times semGive is called */

/* forward declaratiuon */
void syncISR(int);                      /* ISR to unblock syncTask */
void cleanUp ();                        /* cleanup routine */
void syncTask ();                       /* task that needs to be synchronized
                                         * with external events */    

/*****************************************************************************
 * countingSemDemo - demonstrates task synchronization using counting 
 * semaphores. User can also select to use binary semaphore instead of
 * counting semaphore in this demonstration, for comparision between the two 
 * semaphores. 
 *
 *  RETURNS: OK or ERROR
 *
 */

STATUS countingSemDemo 
    (
    char semType          /* counting semaphore type 'c' or binary semaphore 
                           * type 'b'
                           */
    ) 
    {
    switch (semType)
        {
	case 'c':
	case 'C':
	    if ((semId = semCCreate (SEM_Q_PRIORITY, 0)) == NULL)
                {
                perror ("semCCreate");
                return (ERROR);
                }
            break;

        case 'b':
        case 'B':
            if ((semId = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY)) == NULL)
                {
                perror ("semBCreate");
                return (ERROR);
                }
            break;

        default:
            printf ("Unknown semType -- must be 'c' or 'b'\n");
            return (ERROR);
        }

    if ((wdId = wdCreate()) == NULL)
        {
        perror ("wdCreate");
        cleanUp ();
        return (ERROR);
        }


    if ((syncTaskTid = taskSpawn ("tsyncTask", TASK_PRIORITY, 0,TASK_STACK_SIZE,(FUNCPTR) syncTask, 0,0,0,0,0,0,0,0,0,0)) == ERROR)
        {
	perror ("taskSpawn");
	cleanUp();
	return (ERROR);
	}

    /* watchdog simulates hardware interrupts */
    if (wdStart (wdId, TIME_BETWEEN_INTERRUPTS, (FUNCPTR) syncISR, numToGive) 
                  == ERROR)
        {
        perror ("wdStart");
        cleanUp ();
        return (ERROR);
        }

    /* arbitrary delay to allow program to complete before clean up */
    taskDelay (sysClkRateGet()  + ((TASK_WORK_TIME + 2) * numToGive));
	
    cleanUp();
    return (OK);
    }
	

/*****************************************************************************
 *  syncTask - synchronizes with interrupts using counting or binary
 *             semaphores.
 */

void syncTask (void)
    {
    int eventCount = 0;

    FOREVER
        {
	if (semTake (semId, WAIT_FOREVER) == ERROR)
	    {
            perror ("syncTask semTake");
            return;
            }

	/* Do "work" */
	taskDelay (TASK_WORK_TIME);
	semShow (semId,1);

	eventCount++;
	printf ("semaphore taken %d times\n", eventCount);
	}

    }

/*****************************************************************************
 * syncISR - simulates a hardware device which generates interrupts very 
 *           quickly and synchronizes with syncTask using semaphores.
 */
void syncISR 
    (
    int times
    )
    {
    semGive (semId);
    times--;
    if (times > 0)
        wdStart (wdId, TIME_BETWEEN_INTERRUPTS, (FUNCPTR) syncISR, times);
    }

/*****************************************************************************
 * cleanUP - deletes the syncTask, deletes the semaphore and the watchdog timer
 *           previously created by countingSemDemo.
 */
void cleanUp ()
    {
    if (syncTaskTid)
        taskDelete (syncTaskTid);
    if (semId)
        semDelete (semId);
    if (wdId)
        wdDelete (wdId);

    }


