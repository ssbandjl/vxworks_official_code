/* msgQDemo.c - Demonstrates intertask communication using Message Queues */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm  added copyright.
01b,19sep97,ram added include files stdio.h, sysLib.h
		 tested ok
01a,14dec93,ms   written.
*/

/* includes */

#include "vxWorks.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "msgQDemo.h"
#include "sysLib.h"
#include "stdio.h"

/* function prototypes */

LOCAL STATUS producerTask ();           /* producer task */
LOCAL STATUS consumerTask ();           /* consumer task */

/*****************************************************************************
 *  msgQDemo - Demonstrates intertask communication using Message Queues 
 *
 *  DESCRIPTION
 *  Creates a Message Queue for interTask communication between the 
 *  producerTask and the consumerTask. Spawns the producerTask that creates 
 *  messages and sends messages to the consumerTask using the message queue. 
 *  Spawns the consumerTask that reads messages from the message queue. 
 *  After consumerTask has consumed all the messages, the message queue is 
 *  deleted.
 *
 *  RETURNS: OK or ERROR
 *
 *  EXAMPLE
 *
 *  -> sp msgQDemo
 *
 */

STATUS msgQDemo()
    {
    notDone = TRUE;  /* initialize the global flag */

    /* Create the message queue*/
    if ((msgQId = msgQCreate (numMsg, sizeof (struct msg), MSG_Q_FIFO)) 
                                                                      == NULL)
        {
        perror ("Error in creating msgQ");
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

    /* Spwan the consumerTask task */
    if (taskSpawn ("tConsumerTask", CONSUMER_TASK_PRI, 0, TASK_STACK_SIZE, 
                   (FUNCPTR) consumerTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) 
                   == ERROR)
        {
        perror ("consumerTask: Error in spawning demoTask");
        return (ERROR);
        }

    /* polling is not recommended. But used to make this demonstration simple*/
    while (notDone)
        taskDelay (sysClkRateGet ());

    if (msgQDelete (msgQId) == ERROR)
        {
        perror ("Error in deleting msgQ");
        return (ERROR);
        }

    return (OK);
    }


/*****************************************************************************
 *  producerTask - produces messages, and sends messages to the consumerTask 
 *                 using the message queue. 
 *
 *  RETURNS: OK or ERROR
 *
 */

STATUS producerTask (void)
    {
    int count;
    int value;
    struct msg producedItem;           /* producer item - produced data */ 

    printf ("producerTask started: task id = %#x \n", taskIdSelf ());

    /* Produce numMsg number of messages and send these messages */

    for (count = 1; count <= numMsg; count++)
        {
        value = count * 10;   /* produce a value */

        /* Fill in the data structure for message passing */
        producedItem.tid = taskIdSelf ();
        producedItem.value = value;

        /* Send Messages */
        if ((msgQSend (msgQId, (char *) &producedItem, sizeof (producedItem), 
                       WAIT_FOREVER,  MSG_PRI_NORMAL)) == ERROR)
            {
                perror ("Error in sending the message");
                return (ERROR);
            }
        else
            printf ("ProducerTask: tid = %#x, produced value = %d \n", 
                                                      taskIdSelf (), value);
        }

    return (OK);
    }

/*****************************************************************************
 *  consumerTask - consumes all the messages from the message queue. 
 *
 *  RETURNS: OK or ERROR
 *
 */

STATUS consumerTask (void)
    {
    int count;
    struct msg consumedItem;           /* consumer item - consumed data */

    printf ("\n\nConsumerTask: Started -  task id = %#x\n", taskIdSelf());

    /* consume numMsg number of messages */
    for (count = 1; count <= numMsg; count++)
        {
        /* Receive messages */
        if ((msgQReceive (msgQId, (char *) &consumedItem, 
                          sizeof (consumedItem), WAIT_FOREVER)) == ERROR)
            {
            perror ("Error in receiving the message");
            return (ERROR);
            }
        else 
            printf ("ConsumerTask: Consuming msg of value %d from tid = %#x\n",
                            consumedItem.value, consumedItem.tid);
        }

    notDone = FALSE;   /* set the global flag to FALSE to indicate completion*/
    return (OK);
    }



