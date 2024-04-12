/* intGen.c - a program that generates VMEbus interrupt to demonstrate
 *            synchronization  among tasks on multiple CPUs.
 */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01b,17sept97,ram added include files stdio.h, sysLib.h taskLib.h
01a,14jan92,ms   written.
*/

#include "vxWorks.h"
#include "intGen.h"
#include "stdio.h"
#include "sysLib.h"
#include "taskLib.h"

LOCAL int countdown = 25;  /*  countdown value  */
LOCAL int intNum;
LOCAL STATUS intGen ();

/*****************************************************************************
 *  intGenDemo - Demo for generating a VMEbus interrupt
 *
 *  DESCRIPTION
 *
 *  intGen routine  counts down from the initialized countdown value to zero
 *  and then generates VME bus interrupt to synchronize with a task running on
 *  CPU 1.  This program is meant to be run on CPU 0.
 *
 *  The intGen task runs on CPU 0, intSync task runs on CPU 1
 *
 *  CONFIGURATION: You need to set/change the value of INTNUM constant 
 *                 (VMEbus interrupt number to which intSync task's ISR
 *                 is connected) and intNum variable (interrupt number
 *                 for generating VMEbus interrupt).
 *  
 *  RETURNS: OK or ERROR
 *
 *  LIMITATION: Some VMEbus Single Board Computer systems can not generate
 *              VMEbus interrupts. Check your hardware and board specific
 *              BSP documents for VMEbus interrupts support.
 * 
 *  EXAMPLE
 *
 *  The following is an example from CPU 0's VxWorks shell after starting
 *  intSync task from CPU 1:
 *
 *  -> intGenDemo <intSyncTargetArchType>
 * where intSyncTargetArchType is the value for the intSync task's target
 * architecture type. Possible intSyncTargetArchType values and corresponding 
 * architecture types are given below:
 *
 * value     architecture type
 * ----      -----------------
 *  1        68k
 *  2        sparc
 *  3        i960
 *
 * example (to generate a VMEbus interrupt to synchronize with intSync task 
 * running on a Sparc architecture target):
 *
 *  -> intGenDemo 1
 * 
 */

STATUS intGenDemo 
    (
    int intSyncTargetArchType      /* value:
                                    * 1 for 68k architecture targets
                                    * 2 for sparc architecure targets 
                                    * 3 for i960 architecture targets
                                    */
    ) 
    {
    int value;
    int result;

    intNum = 0;

    switch (intSyncTargetArchType)
        {
        case 1: 
            intNum = 255;
            break;

        case 2: 
            intNum = 18;
            break;

        case 3: 
            intNum = 18;
            break;

        default: 
            printf ("Invalid target architecture type \n");
            return (ERROR);
        }

    /* do some useful work here */
    printf ("\n");
    for (value = countdown; value >= 0; value--)
        {
        printf ("%2d\r", value);
        taskDelay (5);
        }
     printf ("\n\nREADY for Synchronization\n\n");
     printf ("CPU 0: Generating VME bus interrupt at level %d and number %d\n\n",
                                                  INTLEVEL, intNum);
     /* generate VMEbus interrupt */
     result = intGen ();

     return (result);
     }


/*****************************************************************************
 *  intGen - generates a VMEbus interrupt
 *
 *  RETURNS: OK or ERROR
 *
 */

STATUS intGen ()
     {     
     /* generate VME bus interrupt.
      * sysBusIntGen requires hardware support for generating a VME bus
      * interrupt.
      */

     if (sysBusIntGen (INTLEVEL, intNum) == ERROR)
         {
         perror ("Failed to generate interrupt");
         return (ERROR);
         }
     else
         {
         printf ("interrupt generated\n\n");
         return (OK);
         }
     }

