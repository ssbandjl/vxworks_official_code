/* To test the time ANSI C library */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,18aug97,whjr
01a,02apr92,smb  
*/

/* INCLUDES */

#include "vxWorks.h"
#include "stdio.h"
#include "time.h"
#include "private/timeP.h"
#include "stdlib.h"
#include "string.h"

#include "test.h"


/*
DESCRIPTION
malloc: This routine allocates a block of memory from the free list.  The 
size of the blcok will be equal to or greater than nBytes.
RETURNS a pointer to the allocated block of memory, or a null pointer
if there is an error.

assert:  If an expression is false (that is, equal to zero) the assert() macro 
writes information about the failed call to standard error in an implementation-
defined format.  It then calls abort().  The diagnostic information includes:
     -the text of the argument
     -the name of the source file (value of preprocessor macro __FILE__)
     -the source line number (value of preprocessor macro __LINE__)

strcmp: This routine compares string s1 to string s2 lexicographically.
RETURNS an integer greater than, equal to, or les than 0, according to whether
s1 is lexcicographically greater than, equal to, or less than s2, respectively.

ctime: This routine converts the calendar time pointed to by timer into local
time in the form of a string.  It is equivalent to:
     asctime (localtime (timer));
RETURNS the pointer returned by asctime() with local broken-down time as the
argument.  

printf: This routine writes output to the stream pointed to by stream, under
control of the string pointed to by format that specifies how subsequent arguments
are converted for output.  If there are insufficient arguments for the format, the
behavior is undefined.  If the format is exhausted while arguments remain, the 
excess arguments are evaluated (as always) but are otherwise ignored.
RETURNS the number of characters trasmitted, or a negative value if an output
error occurred. 

*/


/* A test routine for ctime */
/**********************************************************************
*
*	set global variable TIMEZONE to UTC:UTC:0
*
*/
int testCtimeUTC()
    {
    time_t timeIs;
    char *buffer = malloc(32);
    int result = 0;

    timeIs = (time_t) 0;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"THU JAN 01 00:00:00 1970\n") == 0);

    timeIs = (time_t) 1;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"THU JAN 01 00:00:01 1970\n") == 0);

    timeIs = (time_t) SECSPERMIN;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"THU JAN 01 00:01:00 1970\n") == 0);

    timeIs = (time_t) SECSPERHOUR;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"THU JAN 01 01:00:00 1970\n") == 0);

    timeIs = (time_t) SECSPERDAY ;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"FRI JAN 02 00:00:00 1970\n") == 0);

    timeIs = (time_t) SECSPERDAY * DAYSPERYEAR ;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"FRI JAN 01 00:00:00 1971\n") == 0);

    timeIs = (time_t) SECSPERDAY  * DAYSPERYEAR * 2;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"SAT JAN 01 00:00:00 1972\n") == 0);

    timeIs = (time_t) SECSPERDAY  * (DAYSPERYEAR - 1) ;
    buffer = ctime(&timeIs);
    result += ASSERT(strcmp(buffer,"THU DEC 31 00:00:00 1970\n") == 0);

	if (result == 0)
		fdprintf(2, "SUCCESS testing ctimeUTC\n");
	   else
		fdprintf(2, "Failed %d times testing ctimeUTC\n", result);
	return(result);
    }
