/* recoverExcDemo.c - Demonstrates using signals to allow a task to recover
                      gracefully from an exception                         */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,30sep97,ram added include file usrLib.h for taskIdFigure()
01b,05sep95,ldt  fixed error in pipeDriver
01a,02mar94,ms   modified for VxDemo 
*/

/* includes */
#include "vxWorks.h"
#include "sigLib.h"
#include "signal.h"
#include "setjmp.h"
#include "taskLib.h"
#include "stdio.h"
#include "fioLib.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include "usrLib.h"

/* typedefs */

typedef struct
    {
    VOIDFUNCPTR routine;
    int arg;
    } MSG_REQUEST;

/* defines */
#define TASK_PRI		254
#define PIPE_NAME		"/pipe/server"
#define NUM_MSGS                10

/* globals */
LOCAL int pipeFd;
LOCAL jmp_buf env;

/* function prototypes */
LOCAL void pipeServer ();
LOCAL STATUS serverStart ();
LOCAL STATUS serverSend (VOIDFUNCPTR, int);
STATUS recoverExcDemo ();
extern void i (int);

/*****************************************************************************
 * recoverExcDemo - Demonstrates using signals to allow a task to recover
 *                  gracefully from an exception.
 *
 *
 * DESCRIPTION
 *
 *     tServer task executes functions at low priority (254). serverSend 
 *     funtion is used to send a request to the tServer to execute a 
 *     function at the tServer's  priority.
 *     
 *     First tServer task executes i() system call to print the summary
 *     of tShell task. Next an address exception is caused in tServer
 *     task by trying to execute a funtion at non-aligned word boundry. 
 *     As a result SIGBUS signal is  raised  automatically. Signals 
 *     (setjmp() and longjmp()) are used to allow the tServer task to 
 *     recover gracefully from an address error exception. For more 
 *     information about signals, please refer to sigLib manual pages.
 * 
 * EXAMPLE:
 *
 *     To run this recoverExcDemo from the VxWorks shell do as follows: 
 *     -> ld < recoverExcDemo.o
 *     -> sp (recoverExcDemo)
 *
 */

STATUS recoverExcDemo ()
    {
    printf ("Starting the tServer task\n\n");
    if (serverStart () == ERROR)
        {
        printf ("Error in starting tServer task\n");
        return (ERROR);
        }
    printf ("Running tServer task without causing an exception\n\n"); 
    if (serverSend (i, taskIdFigure ((int)"tShell")) == ERROR)
        {
        printf ("Error in sending request\n");
        return (ERROR);
        }
    taskDelay (5);  /* To get stdout of the function excuted by pipeServer
                     * before executing the next serverSend command 
                     */

    printf ("\n\nCausing an address error exception in tServer task\n\n"); 
    if (serverSend ((VOIDFUNCPTR) 1, 0) == ERROR)
        {
        printf ("Error in sending request\n");
        return (ERROR);
        }
    return (OK);
    } 


/**************************************************************
 * sigHandler -- Returns task to known state.
 *
 *               Called when signal posted.
 *
 */

void sigHandler 
     (
     int sig,     /* signal number */
     int code,    /* additional code */
     SIGCONTEXT *sigContext /* context of task before siganl */
     )
     {
     if (sig == SIGBUS)
         printErr ("\nSignal SIGBUS (bus error) received");
     else 
         printErr ("\nSignal %d received", sig);
     longjmp (env, 1);
     }

/**************************************************************
 * serverStart -- Initializes a server task to execute functons
 *                at a low priority.  Uses pipes as the communication
 *                mechanism.
 *
 * RETURNS: OK or ERROR on failure.
 */

STATUS serverStart ()
    {

    /* Create the pipe device */

    if (pipeDevCreate (PIPE_NAME, NUM_MSGS, sizeof (MSG_REQUEST)) == ERROR)
    {
        perror ("Error in creating Pipe");
	return (ERROR);
    }

	
    /* Open the pipe */

    if ((pipeFd = open (PIPE_NAME, UPDATE, O_RDWR)) == ERROR)
        {
        perror ("Error in opening pipe device"); 
	return (ERROR);
        }
	
    /* Spawn the server task */

    if (taskSpawn ("tServer", TASK_PRI, 0, 5000, (FUNCPTR) pipeServer, 0, 0, 
	            0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
        {
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
    VOIDFUNCPTR routine, /* function to execute at the tServer's priority */ 
    int arg              /* argument for the above function */
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

/**************************************************************
 * pipeServer -- Server task which reads from a pipe and executes
 *               the function passed in the MSG_REQUEST data structure.
 *
 */

LOCAL void pipeServer ()
    {
    MSG_REQUEST msgRequest;
    SIGVEC sig;
    int setjmpReturnValue;

    sig.sv_handler = (VOIDFUNCPTR) sigHandler;
    sig.sv_mask = 0;
    sig.sv_flags = 0;

    setjmpReturnValue = setjmp (env);

    if (setjmpReturnValue == 0)
         {
         /* direct invocation of setjmp */
         /* install signal handler for each of the following signals */
         sigvec (SIGBUS, &sig, NULL);
         sigvec (SIGILL, &sig, NULL);
         sigvec (SIGSEGV, &sig, NULL);
         }
    else
         {
         /* from a call to longjmp */       
         printf (" from %s task (%#x)\n", 
                     taskName (taskIdSelf ()), taskIdSelf());
         }

         while (read (pipeFd, (char *)&msgRequest, sizeof (MSG_REQUEST)) > 0)
             (*msgRequest.routine) (msgRequest.arg);
    }
