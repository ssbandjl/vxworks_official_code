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
#include "string.h"
#include "stdlib.h"
#include "time.h"

#include "private/timeP.h"
#include "test.h"


/*
DESCRIPTION
This file compares the current local time with GM time. 

malloc:
This routine allocates a block of memory from the free list.
The size of the block will be equal to or greater than nBytes.
RETURNS a pointer to the allocated block of memory, or a 
null pointer if there is an error.

assert:
If an expression is false (that is, equal to zero), the assert()
macro writes information about the failed call to standard erro 
in an implementation-defined format.  It then calls abort().  The
diagnostic information includes:

     -the test of the argument
     -the name of the source file (value of preprocessor macro __FILE__)
     -the source line number (value of preprocessor macro __LINE__).
RETURNS N/A.

tmcmp:
This routine compares two structures (member against member).
RETURNS a value of false (that is, equal to zero) if the 
expression are not equal, or returns true otherwise.
*/

int tmcmp(struct tm *tm1, struct tm *tm2)
	{
	if (tm1->tm_sec != tm2->tm_sec) return(FALSE);
	if (tm1->tm_min != tm2->tm_min) return(FALSE);
	if (tm1->tm_hour != tm2->tm_hour) return(FALSE);
	if (tm1->tm_mday != tm2->tm_mday) return(FALSE);
	if (tm1->tm_mon != tm2->tm_mon) return(FALSE);
	if (tm1->tm_year != tm2->tm_year) return(FALSE);
	if (tm1->tm_wday != tm2->tm_wday) return(FALSE);
	if (tm1->tm_yday != tm2->tm_yday) return(FALSE);
	if (tm1->tm_isdst != tm2->tm_isdst) return(FALSE);
		else return(TRUE);
	}

void showTm
	(
	const struct tm *tmptr
	)
	{
	fdprintf(2,"sec %d, min %d, hour %d, %d:%d:%d, day %d, Julian day %d, isdst %d\n",
			tmptr->tm_sec, tmptr->tm_min, tmptr->tm_hour, 
			tmptr->tm_mday, tmptr->tm_mon, tmptr->tm_year, 
			tmptr->tm_wday, tmptr->tm_yday, tmptr->tm_isdst);
	}

/* A test routine for gmtime */
int testGmtime()
    {
    struct tm *tmres = malloc(sizeof(struct tm));
    struct tm *tm2 = malloc(sizeof(struct tm));
    time_t timeIs;
    int result = 0;

    timeIs = (time_t) 0;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 0;
    tm2->tm_hour = 0;
    tm2->tm_mday = 1;
    tm2->tm_mon = 0;
    tm2->tm_year = 70;
    tm2->tm_wday = 4;
    tm2->tm_yday = 0; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

    timeIs = (time_t) SECSPERMIN;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 1;
    tm2->tm_hour = 0;
    tm2->tm_mday = 1;
    tm2->tm_mon = 0;
    tm2->tm_year = 70;
    tm2->tm_wday = 4;
    tm2->tm_yday = 0; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

    timeIs = (time_t) SECSPERHOUR;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 0;
    tm2->tm_hour = 1;
    tm2->tm_mday = 1;
    tm2->tm_mon = 0;
    tm2->tm_year = 70;
    tm2->tm_wday = 4;
    tm2->tm_yday = 0; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

    timeIs = (time_t) SECSPERDAY ;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 0;
    tm2->tm_hour = 0;
    tm2->tm_mday = 2;
    tm2->tm_mon = 0;
    tm2->tm_year = 70;
    tm2->tm_wday = 5;
    tm2->tm_yday = 1; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

    timeIs = (time_t) SECSPERDAY * DAYSPERYEAR ;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 0;
    tm2->tm_hour = 0;
    tm2->tm_mday = 1;
    tm2->tm_mon = 0;
    tm2->tm_year = 71;
    tm2->tm_wday = 5;
    tm2->tm_yday = 0; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

    timeIs = (time_t) SECSPERDAY  * DAYSPERYEAR * 2;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 0;
    tm2->tm_hour = 0;
    tm2->tm_mday = 1;
    tm2->tm_mon = 0;
    tm2->tm_year = 72;
    tm2->tm_wday = 6;
    tm2->tm_yday = 0; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

    timeIs = (time_t) SECSPERDAY  * (DAYSPERYEAR - 1) ;
    tmres = gmtime (&timeIs);
    tm2->tm_sec = 0;
    tm2->tm_min = 0;
    tm2->tm_hour = 0;
    tm2->tm_mday = 31;
    tm2->tm_mon = 11;
    tm2->tm_year = 70;
    tm2->tm_wday = 4;
    tm2->tm_yday = 364; 
    tm2->tm_isdst = 0;
    result += ASSERT(TRUE == tmcmp(tmres, tm2));

	if (result == 0)
		fdprintf(2, "SUCCESS testing gmtime\n");
	   else
		fdprintf(2, "Failed %d times testing gmtime\n", result);
	return(result);
    }
