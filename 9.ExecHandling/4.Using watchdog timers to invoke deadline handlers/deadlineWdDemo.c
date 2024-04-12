/*deadlineWdDemo.c - Demo for invoking deadline handlers using watchdog timers*/

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01a,02mar94,ms   written
*/

/*
 DESCRIPTION
    This program demonstrates using watchdog timers to invoke deadline
    handlers. CoordinatorTask sends data to the organizer. OrganizerTask
    receives data from the coordinatorTask, and resets the coordinatorTask when
    no data is sent by the coordinatorTask in the past five seconds 
    (deadline time). This demonstration program is automatically stopped after 
    twenty seconds.
 
 EXAMPLE
    To run this program from the VxWorks shell do as follows:
 
    -> sp (deadlineWdDemo)
 
*/

/* includes */
#include "vxWorks.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "wdLib.h"
#include "usrLib.h"
#include "sysLib.h"
#include "stdio.h"
#include "logLib.h"

/* defines */

#define   FIVE_SEC          5
#define   TWENTY_SEC        20 
#define   DEADLINE_TIME     FIVE_SEC
#define   NUM_MSGS          10
#define   TASK_STACK_SIZE   20000
#define   PRIORITY          101

/* globals */
LOCAL int countNum;
LOCAL int valueGot;
LOCAL BOOL working;
LOCAL BOOL notDone;
LOCAL WDOG_ID   wdId;
LOCAL MSG_Q_ID msgQId;

/* function prototypes */
LOCAL void deadlineHandler ();    /* deadline handler - watchdog handler 
                                   * routine                 */
LOCAL STATUS coordinatorTask ();  /* sends data to organizer */ 
LOCAL STATUS organizerTask ();    /* receives data from the coordinator and
                                   * resets the coordinator when deadline
                                   * time elapses
                                   */ 
LOCAL void getDataFromDevice ();  /* function that simulates data collection */
/*****************************************************************************
 * deadlineWdDemo - Demo for using watchdog timers to invoke deadline handlers.
 *
 */
STATUS deadlineWdDemo ()
    {
        /* initialize the globals */
        notDone = TRUE;
        working = TRUE;
        countNum = 0;
        valueGot = 0;
    
    /* Create msgQ */
    if ((msgQId = msgQCreate (NUM_MSGS, sizeof (int), MSG_Q_FIFO)) == NULL)
        {
        perror ("Error in creating msgQ");
        return (ERROR);
        }
 
    /* Create watchdog timer */
    if ((wdId = wdCreate ()) == NULL)
        {
        perror ("cannot create watchdog");
        return (ERROR);
        }

        /* Spawn the organizerTask */
        if ((taskSpawn ("tOrgTask", PRIORITY, 0, TASK_STACK_SIZE, 
                          (FUNCPTR) organizerTask, 0, 0, 0, 0, 0, 
                          0, 0, 0, 0, 0))  == ERROR)
        {
        perror ("deadlinWdDemo: Spawning organizerTask failed");
        return (ERROR);
        }
        
        /* Spawn the coordinatorTask */ 
        if ((taskSpawn ("tCordTask", PRIORITY, 0, TASK_STACK_SIZE, 
                        (FUNCPTR) coordinatorTask, 0, 0, 0, 0, 0, 
                        0, 0, 0, 0, 0)) == ERROR)
        {
        perror ("deadlinWdDemo: Spawning coordinatorTask failed");
        return (ERROR);
        }

        /* stop this demo after about 20 seconds*/
        taskDelay (sysClkRateGet () * TWENTY_SEC);
        printf ("\n\nStopping deadlineWdDemo\n");
        notDone = FALSE;
        if (msgQDelete (msgQId) == ERROR)
            {
            perror ("Error in deleting msgQ");
            return (ERROR);
            }
        if (wdCancel (wdId) == ERROR)
            {
            perror ("Error in cancelling watchdog timer"); 
            return (ERROR);
            }
        if (wdDelete (wdId) == ERROR)
            {
            perror ("Error in deleting watchdog timer");
            return (ERROR);
            }

        return (ERROR);
    }        

/*****************************************************************************
 * coordinatorTask - This task is assumed to collect data (countNum) from an 
 *                   external device and sends that data to the organizerTask. 
 *                   This task is constructed to miss the deadline (5 seconds)
 *                   for demonstration purpose. When this task misses deadline
 *                   (5 seconds), it gets reset by the deadlineHandler 
 *                   (using watchdog timers).
 *
 * RETURNS: OK or ERROR
 *
 */
        
STATUS coordinatorTask ()
    {

    FOREVER
        {
        countNum ++;
        if ((msgQSend (msgQId, (char *) &countNum, sizeof (int), NO_WAIT, 
                       MSG_PRI_NORMAL)) == ERROR)
            {
            perror ("Error in sending the message");
            return (ERROR);
            }  
        printf ("\n\ncoordinatorTask: Sent item = %d\n", countNum);
        printf ("coordinatorTask: idle for %d seconds\n", countNum);
        getDataFromDevice ();     /* get data from the device */
        if (notDone == FALSE)
            break;
        }

    return (OK);
    }


/*****************************************************************************
 * organizerTask -   Receives data from the coordinatorTask, and resets the
 *                   coordinatorTask when no data is sent by the 
 *                   coordinatorTask in the past five seconds (deadline time).
 *
 *
 * RETURNS: OK or ERROR
 *
 */
STATUS organizerTask ()
    {

    while (notDone)
        {
        /* If coordinatorTask sends data within the deadline time (5 seconds), 
         * the time elapsed by watchdog timer gets reset to the deadline time,
         * otherwise deadlineHandler is called to reset the coordinatorTask 
         * when no data is sent by the coordinatorTask in the past five 
         * seconds (deadline time).
         */
        if ((wdStart (wdId, sysClkRateGet ()* DEADLINE_TIME, 
                   (FUNCPTR) deadlineHandler, 0)) == ERROR)
            {
            perror ("Error in starting watchdog timer");
            return (ERROR);
            }
        if ((msgQReceive (msgQId, (char *) &valueGot, sizeof (int), 
                                                      WAIT_FOREVER)) == ERROR)
            {
                perror ("Error in receiving the message");
                return (ERROR);
            }  
        else 
            printf ("organizerTask: Received item = %d\n", valueGot);
        }

    return (OK);
    }


/*****************************************************************************
 * deadlineHandler - watchdog timer routine to reset the coordinatorTask 
 *                   when the deadline time expires
 *
 */
 
LOCAL void deadlineHandler ()
    {
    logMsg ("\n\nResetting the co-ordinator on elapse of the deadline time\n",0,0,0,0,0,0);
    countNum = 0;
    }


/*****************************************************************************
 * getDataFromDevice - dummy function that delays for certain amount of time 
 *                     to simulate the data collection for the purpose of 
 *                     demonstration.
 */

LOCAL void getDataFromDevice ()
    {
    taskDelay (sysClkRateGet () * countNum); /* delay this task for countNum
                                              * seconds. */                 
    }
