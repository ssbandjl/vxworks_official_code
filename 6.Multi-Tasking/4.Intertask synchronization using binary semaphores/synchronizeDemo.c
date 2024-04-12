/* synchronizeDemo.c - Demonstrates intertask synchronization using binary 
 *                     semaphores.
 */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm  added copyright.
01b,16sep97,ram included stdio.h,sysLib.h
01a,14feb94,ms   written.
*/

#include "vxWorks.h"
#include "taskLib.h"
#include "semLib.h"
#include "stdio.h"
#include "sysLib.h"

#define  TASK_PRI                 98     /* Priority of the spawned tasks */
#define  TASK_STACK_SIZE          5000   /* stack size for spawned tasks */

LOCAL SEM_ID semId1;            /* semaphore id of binary semaphore 1 */
LOCAL SEM_ID semId2;            /* semaphore id of binary semaphore 2 */
LOCAL int numTimes = 3;         /* Number of iterations */
LOCAL BOOL notDone;                /* flag to indicate completion */
LOCAL STATUS taskA ();
LOCAL STATUS taskB ();

/*****************************************************************************
 *  synchronizeDemo - Demonstrates intertask synchronization using binary 
 *                    semaphores.
 *
 *  DESCRIPTION
 *  Creates two (semId1 and semId2) binary semaphores for intertask
 *  synchronization between two tasks (taskA and taskB). taskA need to execute
 *  an event (event A). On completion of event A, taskB needs to execute
 *  another event (event B). On completion of the event B, taskA needs to 
 *  execute event A. This process needs to be done iteratively. synchronizeDemo
 *  executes this process.
 *
 *  RETURNS: OK or ERROR
 *
 *  EXAMPLE
 *
 *  -> sp synchronizeDemo
 *
 */

STATUS synchronizeDemo ()
    {
    notDone = TRUE;
 
    /* semaphore semId1 is availble  after creation*/
    if ((semId1 = semBCreate (SEM_Q_PRIORITY, SEM_FULL)) == NULL)
        {
        perror ("synchronizeDemo: Error in creating semId1 semaphore");
        return (ERROR);
        }

    /* semaphore semId2 is not available  after creation*/
    if ((semId2 = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY)) == NULL)
        {
        perror ("synchronizeDemo: Error in creating semId2 semaphore");
        return (ERROR);
        }

    /* Spwan taskA*/
    if (taskSpawn ("tTaskA", TASK_PRI, 0, TASK_STACK_SIZE, (FUNCPTR) taskA, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
        {
        perror ("synchronizeDemo: Error in spawning taskA");
        return (ERROR);
        }

    /* Spwan taskB*/
    if (taskSpawn ("tTaskB", TASK_PRI, 0, TASK_STACK_SIZE, (FUNCPTR) taskB, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
        {
        perror ("synchronizeDemo: Error in spawning taskB");
        return (ERROR);
        }
    
    /* Polling is not recommended. But used for simple demonstration purpose */
    while (notDone) 
        taskDelay (sysClkRateGet());  /* wait here until done */
        /* Delete the created semaphores */
    if (semDelete (semId1) == ERROR)
        {
        perror ("syncronizeDemo: Error in deleting semId1 semaphore");
        return (ERROR);
        }
    if (semDelete (semId2) == ERROR)
       {
       perror ("syncronizeDemo: Error in deleting semId1 semaphore");
       return (ERROR);
       }
    printf ("\n\n synchronizeDemo now completed \n");
    
    return (OK);
    }

/*****************************************************************************
 *  taskA - executes event A first and wakes up taskB to excute event B next
 *          using binary semaphores for synchronization.
 *
 *  RETURNS: OK or ERROR
 *
 */

LOCAL STATUS taskA ()
    {
    int count;

    for (count = 0; count < numTimes; count++)
        {
        if (semTake (semId1, WAIT_FOREVER) == ERROR)
            {
            perror ("taskA: Error in semTake");
            return (ERROR);
            } 
        printf ("taskA: Started first by taking the semId1 semaphore - %d times\n",
                (count + 1));
        printf("This is task  <%s> : Event A now done\n", taskName (taskIdSelf()));
        printf("taskA: I'm done, taskB can now proceed; Releasing semId2 semaphore\n\n");
        if (semGive (semId2) == ERROR)
            {
            perror ("taskA: Error in semGive");
            return (ERROR);
            }
        }
        return (OK);
    }


/*****************************************************************************
 *  taskB - executes event B first and wakes up taskA to excute event A next
 *          using binary semaphores for synchronization.
 *
 *  RETURNS: OK or ERROR
 *
 */
LOCAL STATUS taskB()
    {
    int count;

    for (count = 0; count < numTimes; count++)
        {
        if (semTake (semId2,WAIT_FOREVER) == ERROR)
            {
            perror ("taskB: Error in semTake");
            return (ERROR);
            } 
        printf ("taskB: Synchronized with taskA's release of semId2 - %d times\n",
                (count + 1 ));
        printf("This is task  <%s> : Event B now done\n", taskName (taskIdSelf()));
        printf("taskB: I'm done, taskA can now proceed; Releasing semId1 semaphore\n\n\n");
        if (semGive (semId1) == ERROR)
            {
            perror ("taskB: Error in semGive");
            return (ERROR);
            }
        }
        notDone = FALSE;
        return (OK);
    }

