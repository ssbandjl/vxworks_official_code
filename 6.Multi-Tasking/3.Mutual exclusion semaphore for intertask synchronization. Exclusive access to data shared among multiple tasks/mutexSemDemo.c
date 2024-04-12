/* mutexSemDemo.c -  Demonstrate the usage of the mutual exclusion semaphore 
 *                   for intertask synchronization and obtaining exclusive 
 *                   access to a data structure shared among multiple tasks.
 */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm  added copyright.
01b,16sep97,ram included logLib.h, sysLib.h, stdio.h
		 The arguments to logMsg are required arguments(6)
		 Since there were fewer than 6 arguments the remaining
		 have been filled up with zeros.

01a,14jan94,ms   written.
*/

#include "vxWorks.h"
#include "semLib.h"
#include "taskLib.h"
#include "mutexSemDemo.h"
#include "logLib.h"
#include "sysLib.h"
#include "stdio.h"

LOCAL STATUS protectSharedResource ();      /* protect shared data structure */
LOCAL STATUS releaseProtectedSharedResource (); /* release protected access */
LOCAL STATUS producerTask ();               /* producer task */
LOCAL STATUS consumerTask ();               /* consumer task */
LOCAL struct shMem shMemResource;           /* shared memory structure */
LOCAL SEM_ID mutexSemId;                    /* mutual exclusion semaphore id*/
LOCAL BOOL notFinished;                     /* Flag that indicates the 
                                             * completion */

/*****************************************************************************
 *  mutexSemDemo - Demonstrate the usage of the mutual exclusion semaphore 
 *                 for intertask synchronization and obtaining exclusive 
 *                 access to a data structure shared among multiple tasks.
 *
 *  DESCRIPTION
 *  Creates a mutual exclusion semaphore for intertask syncronization 
 *  between the producerTask and the consumerTask. Both producerTask and
 *  consumerTask access and manipulate the global shared memory data 
 *  structure simultaneously. To avoid corruption of the global shared
 *  memory data structure mutual exclusion semaphores are used. 
 *
 *  Spawns the producerTask that produces the message and puts the message
 *  in the global shared data structure. Spawns the consumerTask that 
 *  consumes the message from the global shared data structure and
 *  updates the status field to CONSUMED so that producerTask can put
 *  the next produced message in the global shared data structure. 
 *  After consumerTask has consumed all the messages, the mutual exclusion
 *  semaphore is deleted.
 *
 *  RETURNS: OK or ERROR
 *
 *  EXAMPLE
 *
 *  -> sp mutexSemDemo
 *
 */
 
STATUS mutexSemDemo()
    {
    notFinished = TRUE;  /* initialize the global flag */

 
    /* Create the mutual exclusion semaphore*/
    if ((mutexSemId = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE
                                          | SEM_INVERSION_SAFE)) == NULL)
        {
        perror ("Error in creating mutual exclusion semaphore");
        return (ERROR);
        }
 
 
    /* Spwan the consumerTask task */
    if (taskSpawn ("tConsumerTask", CONSUMER_TASK_PRI, 0, TASK_STACK_SIZE, 
                   (FUNCPTR) consumerTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) 
                   == ERROR)
        {
        perror ("consumerTask: Error in spawning demoTask");
        return (ERROR);
        }

     /* Spwan the producerTask task */
    if (taskSpawn ("tProducerTask", PRODUCER_TASK_PRI, 0, TASK_STACK_SIZE, 
                   (FUNCPTR) producerTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) 
                   == ERROR)
        {
        perror ("producerTask: Error in spawning demoTask");
        return (ERROR);
        } 
 
    /* Polling is not recommended. But used for making this demonstration 
     * simple */

    while (notFinished)
        taskDelay (sysClkRateGet ());

    /* When done delete the mutual exclusion semaphore*/ 
    if (semDelete (mutexSemId) == ERROR)
       {
       perror ("Error in deleting mutual exclusion semaphore");
       return (ERROR);
       }
 
     return (OK);
     }

/*****************************************************************************
 *  producerTask - produce the message, and write the message to the global 
 *                 shared data structure by obtaining exclusive access to 
 *                 that structure which is shared with the consumerTask. 
 *
 *  RETURNS: OK or ERROR
 *
 */

STATUS producerTask ()
    {
    int count = 0;
    int notDone = TRUE;

    while (notDone)
        {

        /* Produce NUM_ITEMS, write each of these items to the shared
         * global data structure.
         */
 
        if (count < NUM_ITEMS)
            {

            /* Obtain exclusive access to the global shared data structure */
            if (protectSharedResource() == ERROR)
                return (ERROR);

            /* Access and manipulate the global shared data structure */
            if (shMemResource.status == CONSUMED)
                { 
                count++;
                shMemResource.tid = taskIdSelf ();
                shMemResource.count = count;
                shMemResource.status = PRODUCED;
                }
            /* Release exclusive access to the global shared data structure */
            if (releaseProtectedSharedResource () == ERROR)
                return (ERROR);

            logMsg ("ProducerTask: tid = %#x, producing item = %d\n", 
                                                       taskIdSelf (), count,0,0,0,0);
            taskDelay (sysClkRateGet()/6); /* relingiush the CPU so that 
                                            * consumerTask can access the 
                                            * global shared data structure. 
                                            */
            }
        else 
            notDone = FALSE;
        }

    return (OK);
    }


/*****************************************************************************
 *  consumerTask -  consumes the message from the global shared data 
 *                  structure and updates the status filled to CONSUMED 
 *                  so that producerTask can put the next produced message 
 *                  in the global shared data structure. 
 *
 *  RETURNS: OK or ERROR
 *
 */

STATUS consumerTask ()
    {
    int notDone = TRUE;

    /* Initialize to consumed status */
    if (protectSharedResource() == ERROR)
        return (ERROR);
    shMemResource.status = CONSUMED;
    if (releaseProtectedSharedResource () == ERROR)
        return (ERROR);

    while (notDone) 
        {
        taskDelay (sysClkRateGet()/6);     /* relingiush the CPU so that 
                                            * producerTask can access the 
                                            * global shared data structure. 
                                            */

        /* Obtain exclusive access to the global shared data structure */
        if (protectSharedResource() == ERROR)
            return (ERROR);

        /* Access and manipulate the global shared data structure */
        if ((shMemResource.status == PRODUCED) && (shMemResource.count > 0)) 
            {
            logMsg ("ConsumerTask: Consuming item = %d from tid = %#x\n\n",
                shMemResource.count, shMemResource.tid,0,0,0,0);
            shMemResource.status = CONSUMED;
            }
        if (shMemResource.count >= NUM_ITEMS)
            notDone = FALSE;

        /* Release exclusive access to the global shared data structure */
        if (releaseProtectedSharedResource () == ERROR)
            return (ERROR);


        }

    notFinished = FALSE;
    return (OK);
    }


/*****************************************************************************
 *  protectSharedResource -  Protect access to the shared data structure with 
 *                           the mutual exclusion  semaphore.
 *
 *  RETURNS: OK or ERROR
 *
 */
LOCAL STATUS protectSharedResource ()
    {
    if (semTake (mutexSemId, WAIT_FOREVER) == ERROR)
        {
        perror ("protectSharedResource: Error in semTake");
        return (ERROR);
        }
    else
        return (OK);
    }

/*****************************************************************************
 *  releaseProtectedSharedResource -  Release the protected access to the 
 *                                    shared data structure using the mutual 
 *                                    exclusion semaphore
 *
 *  RETURNS: OK or ERROR
 *
 */
LOCAL STATUS releaseProtectedSharedResource ()
    {
    if (semGive (mutexSemId) == ERROR)
        {
        perror ("protectSharedResource: Error in semTake");
        return (ERROR);
        }
    else
        return (OK);
    }






