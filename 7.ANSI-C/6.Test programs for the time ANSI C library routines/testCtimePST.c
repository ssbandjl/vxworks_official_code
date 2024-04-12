/* To test the time ANSI C library */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,13aug97,whjr     Documentation and Modified
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
DESCRIPTION:
ctime: This routine converts the calendar time pointed to
by timer into local time in the from of a string.  It is
equivalent to:
    asctime (localtime (timer));
Returns the pointer returned by asctime() with local broken
-down time as the argument.

strcmp: The routine compares up to n characters of string
s2 lexicographically. Returns an integer greater than, equal
to, or less than 0, according to whether s1 is lexicographically
greater than, equal to, or less than s2, respectively. 

malloc: This routine allocates a blcok of memory from the free
list.  The size of the block will be equal to or greater than nBytes.
RETURNS a pointer to the allocated block of memory, or a null
pointer if there is an error.
*/

/* A test routine for ctime */

/*************************************************************
*
*	set global variable TIMEZONE to PST:PST:480
*
*/


int testCtimePST()
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
		fdprintf(2, "SUCCESS testing ctimePST\n");
	   else
		fdprintf(2, "Failed %d times testing ctimePST\n", result);
	return(result);
    }
