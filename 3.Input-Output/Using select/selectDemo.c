/* selectDemo.c - a program to demonstrate the usage of select routine */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01b,21aug97,ram	 tested OK
01a,20apr92,ms   written
*/

/*
DESCRIPTION

    This program demonstrates the usage of the select routine. The selectDemo 
    task writes messages to pipe devices. The selectOnReadFds task blocks on 
    the select routine and then reads a message only from the pipe devices 
    that are ready for reading.

    To run this program from the VxWorks shell do as follows:

    -> ld < selectDemo.o
    value = 0 = 0x0
    -> selectDemo

*/

#include "vxWorks.h"
#include "taskLib.h"
#include "selectLib.h"
#include <string.h> 
#include <ioLib.h>  
#include <stdio.h>  
#include <fioLib.h>
#include <pipeDrv.h> 
#include <sysLib.h> 

#define BUFFERSIZE     25                               /* size of the buffer */
#define NUM_FDS     2                                /* no. of fds used    */
#define ONE_SEC     1
#define STRING1 "SelectDemo: Message #1" 
#define STRING2 "SelectDemo: Message #2"
#define STRING3 "Quitting SelectDemo"
#define MAX_NUM_MSG 2                  /* max. number of messages in pipe */
#define MSG_SIZE    100                           /* size of each message */
LOCAL int inFd1;                   /* fd for /pipe/1 device */  
LOCAL int inFd2;                   /* fd for /pipe/2 device */


/*******************************************************************************
* selectOnReadFds - selectOnReadFds task
*
* This task blocks on a select call until one or more pipe devices become ready
* for reading, and then reads message only from the pipe devices that are ready
* for reading.
*
* RETURNS: OK or ERROR
*/

STATUS selectOnReadFds ()
    {
    char receiveString [BUFFERSIZE];      /* buffer for the received message */
    struct fd_set readFds;
    struct fd_set saveFds;
    int width;
    int bytesRead;
    int numFds;                                          /* no. of fds */
    int notFinished = TRUE;

    width = 0;
    FD_ZERO (&saveFds);    /* initialize the set - all bits off */

    /* The width argument is the maximum file descriptor to 
     * be tested, plus one. The descriptors 0,1, up through 
     * and including width -1 are tested.  
     */
    FD_SET (inFd1, &saveFds); /* Turn on bit for inFd1 */
    FD_SET (inFd2, &saveFds); /* Turn on bit for inFd2 */     
    width = (inFd1 > inFd2) ? inFd1 : inFd2;
    width++; 

    while (notFinished)
        {
        readFds = saveFds;
        printf ("selectOnReadFds: Number of bits (in fd_set struct) to be tested = %d\n", width);

        /* Pend on multiple file descriptors indefinitely using select
         * until one or more file descriptors become ready for reading.
         */

        if ((numFds = select (width, &readFds, NULL, NULL, NULL)) == ERROR)
            {
            perror (" ERROR in select");   
            return (ERROR);
            }
        else 
            printf ("selectOnReadFds: Number of file descriptors ready for reading = %d\n", numFds);

        if (FD_ISSET (inFd1, &readFds))
            {
            /* /pipe/1 is ready to be read from */
            bzero (receiveString, BUFFERSIZE);
            if ((bytesRead = read (inFd1, receiveString, BUFFERSIZE)) > OK)
                printf ("selectOnReadFds: Message read from /pipe/1 device - \"%s\"\n",(char *) receiveString);
            if (strcmp (receiveString, STRING3) == 0)
                notFinished = FALSE;
            }    
        else 
            printf ("selectOnReadFds: The fd inFd1 of /pipe/1 dev. is not READY for reading\n");

        if (FD_ISSET (inFd2, &readFds))
            {
            /* /pipe/2 is ready to be read from */
            bzero (receiveString, BUFFERSIZE);
            if ((bytesRead = read (inFd2, receiveString, BUFFERSIZE)) > OK)
                printf ("selectOnReadFds: Message read from /pipe/2 device - \"%s\"\n", (char *) receiveString);
            }    
        else 
            printf ("selectOnReadFds: The fd inFd2 of /pipe/2 dev. is not READY for reading\n");
        printf ("\n");
        }

        return (OK);
    }

/******************************************************************************
 *
 * selectDemoInit - Initialize the pipe devices 
 *
 * This routine initialize the pipe devices for both writing and reading 
 * messages.
 *
 */

void selectDemoInit ()
    {

    if (pipeDevCreate ("/pipe/1",MAX_NUM_MSG, MSG_SIZE) == ERROR)
        perror ("selectDemoInit: error in creating\"/pipe/1\"");
    else 
        printf ("\"/pipe/1\" pipe device created\n");

    if (pipeDevCreate ("/pipe/2", MAX_NUM_MSG, MSG_SIZE) == ERROR)
        perror ("selectDemoInit: error in creating \"/pipe/2\"");
     else 
        printf ("\"/pipe/2\" pipe device created\n\n");

     if ((inFd1 = open ("/pipe/1", UPDATE, 0644)) == ERROR) /* @@@ ldt */
        perror (" Error opening file \"/pipe/1\"") ;
     else 
        printf ("Value of the fd for /pipe/1 = %d \n", inFd1);

     if ((inFd2 = open ("/pipe/2", UPDATE, 0644)) == ERROR) /* @@@ ldt */
        perror (" Error opening file \"/pipe/1\"") ;
     else 
        printf ("Value of the fd for /pipe/2 = %d \n\n", inFd2);

    }    

/******************************************************************************
 *
 * selectDemo - demonstrates the usage of select
 *
 * This routine demonstrates the usage of select on readable pipe devices.
 * selectDemo routine initializes the pipe devices and spawns the 
 * selectOnReadFds task. Then messages are written to the pipe devices. 
 * selectOnReadFds task blocks on the select routine and then reads message 
 * only from the pipe devices that are ready for reading.
 *
 * RETURNS: OK or ERROR
 */

STATUS selectDemo ()
    {
    char input[80];
    int makeFdsReady;
    int notDone = TRUE;     /* Boolean variable for the demo status */  

    selectDemoInit ();      /* Initialize the pipe devices*/

    /* Spawn the selectOnReadFds task */
    if ((taskSpawn ("selectOnReadFds", 100, VX_SUPERVISOR_MODE | VX_STDIO,
           20000, (FUNCPTR) selectOnReadFds, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)) 
                                                                      == ERROR)
        {
        perror ("selectDemo: Spawning selectOnReadFds task failed");
        return (ERROR); 
        }

    while (notDone)
        {
        /* relinguish the CPU for sometime so that selectOnReadFds 
         * task can be executed. 
         */
        taskDelay (sysClkRateGet () * ONE_SEC);

        makeFdsReady = NONE;
        printf ("0 : To quit this Demo\n");
        printf ("1 : To write a message to \"/pipe/1\"\n");
        printf ("2 : To write a message to \"/pipe/2\"\n");
        printf ("3 : To write a message to \"/pipe/1\" and \"/pipe/2\"\n");
        printf ("Enter your choice : ");
        gets (input);
        sscanf (input, "%d", &makeFdsReady);
        printf ("\n");    

        switch (makeFdsReady)
            {
            case 1:  /* To write a msg to /pipe/1 */
                printf ("Writing the message \"%s\" to /pipe/1 device\n", (char *) STRING1);
                if (write (inFd1, STRING1, sizeof (STRING1)) == ERROR)
                    perror ("selectDemo: Writing msg to pipe 1 failed");
                break;

            case 2:  /* To write a msg to /pipe/2 */
                printf ("Writing the message \"%s\" to /pipe/2 device\n", (char *) STRING2);
                if (write (inFd2, STRING2, sizeof (STRING2)) == ERROR)
                    perror ("selectDemo: Writing msg to pipe 2 failed");
                break;         

            case 3:  /* To write a msg to /pipe/1 and /pipe/2 */
                printf ("Writing the message \"%s\" to /pipe/1 device\n", (char *) STRING1);
                if (write (inFd1, STRING1, sizeof (STRING1)) == ERROR)
                    perror ("selectDemo: Writing msg to pipe 1 failed");
                printf ("Writing the message \"%s\" to /pipe/2 device\n", (char *) STRING2);
                if (write (inFd2, STRING2, sizeof (STRING2)) == ERROR)
                    perror ("selectDemo: Writing msg to pipe 2 failed");
                break;         

            case 0:  /* To write a quitting message to /pipe/1 */
                notDone = FALSE;
                printf ("Writing the message \"%s\" to /pipe/1 device\n", (char *) STRING3);
                if (write (inFd1, STRING3, sizeof (STRING3)) == ERROR)
                    perror ("selectDemo: Writing msg to pipe 1 failed");
                taskDelay (sysClkRateGet () * ONE_SEC);
                break;

            default :
                printf ("Unrecognized Choice \n");
                break;
            }
                
            printf ("\n");
        }

        close (inFd1);
        close (inFd2); 
        return (OK);
    }    
