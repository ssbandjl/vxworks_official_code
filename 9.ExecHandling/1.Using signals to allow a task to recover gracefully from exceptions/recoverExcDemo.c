/* recoverExcDemo.c - Demonstrates using signals to allow a task to recover
                      gracefully from an exception                         */


/*
01b,05Sep95,ldt  fixed error in pipeDriver
01a,02mar94,ms   modified for VxDemo 
*/

#include "vxWorks.h"
#include "sigLib.h"
#include "signal.h"
#include "setjmp.h"
#include "taskLib.h"
#include "stdio.h"
#include "fioLib.h"
#include "ioLib.h"
#include "pipeDrv.h"

#define NULL_ADDR 0

typedef struct
    {
    VOIDFUNCPTR routine;
    int arg;
    } MSG_REQUEST;

#define TASK_PRI		254
#define PIPE_NAME		"/pipe/server"
#define NUM_MSGS                10

/* setjmp buffer */
LOCAL jmp_buf env;

extern void i (int);
STATUS recoverExcDemo ();

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

     switch (sig)
     {
         case SIGBUS: 
         printErr ("\nSignal SIGBUS (bus error) received\n");
         break;

         case SIGILL: 
         printErr ("\nSignal SIGILL received\n");
         break;

         case SIGSEGV:
         printErr ("\nSignal SIGSEGV received\n");
         break;
}

     longjmp (env, sig);
     }

/**************************************************************
 *
 */
STATUS demo_raise ()
{
    printf ("causing bus error\n");
    /* Deliberately raise bus error */
    (* (FUNCPTR) NULL_ADDR)();
    return OK;
}

/**************************************************************
 *
 */

STATUS demo_recover ()
{
    int val = 0;
    SIGVEC sig;
    int setjmpReturnValue;

    sig.sv_handler = (VOIDFUNCPTR) sigHandler;
    sig.sv_mask = 0;
    sig.sv_flags = 0;

    val = 50;

    printf ("demo_recover...\n");

    setjmpReturnValue = setjmp (env);

    if (setjmpReturnValue == 0)
    {
         /* direct invocation of setjmp */
         /* install signal handler for each of the following signals */
         sigvec (SIGBUS, &sig, NULL);
         sigvec (SIGILL, &sig, NULL);
         sigvec (SIGSEGV, &sig, NULL);
         printf ("demo_recover...setup sigvec\n");
    }
    else
    {
        /* from a call to longjmp */      
        printf ("recover from signal %d. val = %d \n", setjmpReturnValue, val);
        return OK;
    }
    /* raise the exception */
    demo_raise ();
    return OK;
}

STATUS recoverDemo ()
{
    demo_recover ();
    return OK;
}
