/* pipeServer.c - Demonstrates intertask communication using pipes */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm  added copyright.
01b,19sep97,ram need to add INCLUDE_PIPE in configAll.h
		 added include files stdio.h,ioLib.h,pipeDrv.h
01a,11mar94,ms   cleaned up for VxDemo
*/


/* DESCRIPTION
 *
 * Demonstrates intertask communication using pipes.
 *
 * serverStart initializes a server task to execute functions
 * at a low priority and uses pipes as the communication
 * mechanism. 
 *
 * serverSend sends a request to the server to execute a 
 * function at the server's  priority using pipe device.
 * Usage : serverSend (<function>, <argument of the function>)
 *
 * EXAMPLE
 * To run this demo from the Wind shell do as follows:
 *
 * -> serverStart
 * value = 0 = 0x0
 * -> serverSend (printf,"Hello World. \n")
 * value = 0 = 0x0
 *
 * NOTE: A host function such as i() cannot be used as the function
 *       argument to serveSend.
 *	 eg: -> serverSend(i,"tShell") will not work
 *       However if you wish to do that then make sure to add
 *	 INCLUDE_CONFIGURATION_5_2 and INCLUDE_DEBUG in configAll.h
*/

/* includes */

#include "vxWorks.h"
#include "taskLib.h"
#include "stdio.h"
#include "ioLib.h"
#include "pipeDrv.h"


typedef struct
    {
    VOIDFUNCPTR routine;
    int arg;
    } MSG_REQUEST; 	                        /* message structure */


#define TASK_PRI		254             /* tServers task's priority */
#define TASK_STACK_SIZE         5000            /* tServer task's stack size */
#define PIPE_NAME		"/pipe/server"  /* name of the pipe device */
#define NUM_MSGS                10     /* max number of messages in the pipe */

LOCAL int pipeFd;                     /* File descriptor for the pipe device */
LOCAL void pipeServer ();             /* server task */


/***********************************************************************
 * serverStart -- Initializes a server task to execute functons
 *                at a low priority.  Uses pipes as the communication
 *                mechanism.
 *
 * RETURNS: OK or ERROR on failure.
 */

STATUS serverStart ()
    {

    if (pipeDevCreate (PIPE_NAME, NUM_MSGS, sizeof (MSG_REQUEST)) == ERROR)
        {
        perror ("Error in creating pipe"); /* print error if pipe is already 
                                            * created, but do not return */
        }

    /* Open the pipe */
    if ((pipeFd = open (PIPE_NAME, UPDATE, 0)) == ERROR)
        {
        perror  ("Error in opening pipe device");
        return (ERROR);
        }
	
    /* Spawn the server task */
    if (taskSpawn ("tServer", TASK_PRI, 0, TASK_STACK_SIZE, 
                   (FUNCPTR) pipeServer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) 
                   == ERROR)
        {
        perror ("Error in spawning tServer task");
	close (pipeFd);
	return (ERROR);
	}
	
    return (OK);
    }

/**************************************************************
 * serverSend -- Sends a request to the server to execute a 
 *               function at the server's  priority.
 *
 * RETURNS: OK or ERROR on failure.
 */

STATUS serverSend 
    (
    VOIDFUNCPTR routine,              /* name of the routine to execute */
    int arg                           /* argument of the routine */        
    )
    {
    MSG_REQUEST msgRequest;
    int status;

    /* Initialize the message structure */
    msgRequest.routine = routine;
    msgRequest.arg     = arg;

    /* Send the message and return the results */
    status = write (pipeFd, (char *)&msgRequest, sizeof (MSG_REQUEST));

    return ((status == sizeof (MSG_REQUEST)) ? OK : ERROR);
    }

/*************************************************************************
 * pipeServer -- Server task which reads from a pipe and executes
 *               the function passed in the MSG_REQUEST data structure.
 *
 */

LOCAL void pipeServer ()
	{
	MSG_REQUEST msgRequest;

	while (read (pipeFd, (char *)&msgRequest, sizeof (MSG_REQUEST)) > 0)
		(*msgRequest.routine) (msgRequest.arg);
		}








