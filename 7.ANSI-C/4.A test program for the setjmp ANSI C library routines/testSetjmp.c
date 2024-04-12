/* To test the setjmp ANSI C library */

/* Copyright 1991 Wind River Systems, Inc. */

/*
modification history
--------------------
02a,06aug97,ram		modified and documented
01a,02apr92,smb  
*/

/*
DESCRIPTION
Test routines for setjmp and longjmp.
These functions are useful for dealing with errors and interrupts
encountered in a low-level subroutine of a program.

setjmp: saves the calling environment in a jmp_buf argument for
later use by longjmp.
int setjmp(jmp_buf env);

longjmp: performs non-logical got by restoring the saved environment.
void longjmp(jmp_buf env, int val);
After longjmp() is completed, program execution continues as if the
corresponding call of setjmp() just returned the value val.

jmpbuf: an array type suitable for holding the information needed to
restore a calling environment.

RETURN VALUES
setjmp: From a direct invocation setjmp returns a zero. Froms a call
to longjmp, it returns a  non zero value specified as an argument to longjmp().

longjmp: This routine does not return to its caller. Instead,
it causes setjmp() to return val,unless val is zero; in that case
setjmp() returns 1.

INCLUDE setjmp.h
*/

#include <vxWorks.h>
#include <stdio.h>
#include <setjmp.h>

#include "test.h"

static int ctr;
static jmp_buf b0; 


/* A test routine for setjmp*/

static void jmpto
	(
	int n
	)
	{
	longjmp(b0, n);  /* resets the environment according to the */
			 /* environment varibles */
			 /* as saved in the jmp_buf b0 */
	}

static char *stackptr(void)
	{
	char ch;

	return(&ch);
	}

/* This test tests repeatedly for stack creep */

static int tryit(void)
	{
	jmp_buf b1;
	char *sp = stackptr();
	int res = 0;
	int wait;
	ctr = 0;
	switch (setjmp(b0))  /* saves the current environment */
		{
	case 0: 
		printf("In case 0 \n");
		res = ASSERT(sp == stackptr());
		res = ASSERT(ctr == 0);
		++ctr;

		printf("calling function jmto which makes a call to longjmp\n");
		printf("with its second arg = 0 which causes setjmp to return a 1.\n"); 
		printf(" \n PRESS ENTER TO CONTINUE ...\n");
		wait=getchar();
		jmpto(0);  /* calling function jmto which makes a call to*/ 
			   /* longjmp with its second arg = 0 */ 
			   /* which causes setjmp to return a 1. */
		break;
	case 1:
		printf("In case 1 \n");
		res = ASSERT(sp == stackptr());
		res = ASSERT(ctr == 1);
		++ctr;
		printf("calling function jmpto which makes a call to longjmp\n");
		printf("with its second arg = 2 which causes setjmp to return a 2.\n"); 
		printf(" \n PRESS ENTER TO CONTINUE ...\n");
		wait=getchar();
		jmpto(2); /* call to longjmp with its second arg = 2 */
			  /* setjmp returns 2. */
		break;
	case 2:
		printf("In case 2 \n");
		res = ASSERT(sp == stackptr());
		res = ASSERT(ctr == 2);
		++ctr;
		switch (setjmp(b1))
			{
		case 0:
			printf("In case 0 of case 2\n");
			res = ASSERT(sp == stackptr());
			res = ASSERT(ctr == 3);
			++ctr;
			printf("calling function jmpto which makes a call to longjmp\n");
			printf("with its second arg = -7  which causes setjmp to return a -7.\n"); 
			printf(" \n PRESS ENTER TO CONTINUE ...\n");
			wait=getchar();
			longjmp(b1, -7); /* causes setjmp to return -7. */
			break;
		case -7:
			printf("In case -7 of case 2 \n");
			res = ASSERT(sp == stackptr());
			res = ASSERT(ctr == 4);
			++ctr;
			printf("calling function jmpto which makes a call to longjmp\n");
			printf("with its second arg = 3  which causes setjmp to return a 3.\n"); 
			printf(" \n PRESS ENTER TO CONTINUE ...\n");
			wait=getchar();
			jmpto(3);
		case 5:
			printf("In case 5 of case 2  \n");
			printf("returning execution to main program.\n");
			printf(" \n PRESS ENTER TO CONTINUE ...\n");
			wait=getchar();
			return(13);
		default:
			return(0);
			}
	case 3:
		printf("In case 3 \n");
		printf("longjmp(b1, 5):- makes the corresponding call of setjmp() to return a value of 5, i.e,\n");
		printf("the program execution goes back to the setjmp() function in case 2 and then causes the switch to case 5.\n"); 
		longjmp(b1, 5); /* makes the corresponding call of setjmp() to return a */
				/* value of 5, i.e, the program execution goes back to  */
				/* the setjmp() function in case 2 and then causes the  */
				/* switch to case 5. */


		break;
		}
	return (-1);
	}

/***************************************************************************
 testsetjmp: To test setjmp and longjmp

 RETURN: OK on success else  gives an ASSERTION ERROR.
*/

int testsetjmp(void)
    {
    int result = 0;

    result += ASSERT(tryit() == 13); /* verifying that the function returns a value=13. */
#if ALL_OUTPUTS
    fdprintf(2, "size of jmp_buf = %u\n", sizeof(jmp_buf));
#endif

	if (result == 0)
		fdprintf(2, "SUCCESS testing setjmp\n");
	   else
		fdprintf(2, "Failed %d times testing setjmp\n", result);
	return(result);
    }

