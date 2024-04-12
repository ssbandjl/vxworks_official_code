/* To test the time ANSI C library */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,19aug97,whjr
01a,02apr92,smb  
*/


/* INCLUDES */

#include <vxWorks.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"


/*
DESCRIPTION 
mktime() converts the time represented by the  tm  structure
pointed  to  by  timeptr into a calendar time (the number of
seconds since 00:00:00 UTC, January 1, 1970).
RETURNS the specified calendar time.  If the calendar time 
cannot be represented, the function returns the value (time_t) -1.
ctime:
This routine converts the calendar time pointed to by timer into local
time in the form of a string.  It is equivalent to:
     asctime (localtime (timer));
RETURNS the pointer returned by asctime() with local broken-down
time as the argument.

assert:
If an expression is false (that is, equal to zero) the assert() macro 
writes information about the failed call to standard error in an 
implementation-defined format.  It then calls abort().  The diagnostic
information includes:
    -the text of the argument
    -the name of the source file (value of preprocessor macro __FILE__)
    -the source line number (value of preprocessor macro __LINE__)

strcmp: This routine compares string s1 to string s2 lexicographically.
RETURNS an integer greater than, equal to, or les than 0, according to whether
s1 is lexcicographically greater than, equal to, or less than s2, respectively.

*/

/* A test routine for mktime */
int testMktime()
    {
    struct tm *tmptr; 
    time_t restime;
    char *buffer = malloc(32);
    int result = 0;

    tmptr = (struct tm *) malloc(sizeof(*tmptr));

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 9;
    tmptr->tm_mon = 2;
    tmptr->tm_year = 92;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "MON MAR 09 00:00:00 1992\n") == 0);

    tmptr->tm_sec = 60;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 1;
    tmptr->tm_mon = 0;
    tmptr->tm_year = 70;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "THU JAN 01 00:01:00 1970\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 60;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 1;
    tmptr->tm_mon = 0;
    tmptr->tm_year = 70;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "THU JAN 01 01:00:00 1970\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 24;
    tmptr->tm_mday = 1;
    tmptr->tm_mon = 0;
    tmptr->tm_year = 70;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "FRI JAN 02 00:00:00 1970\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 50;
    tmptr->tm_mon = 0;
    tmptr->tm_year = 80;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "TUE FEB 19 00:00:00 1980\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 1;
    tmptr->tm_mon = 12;
    tmptr->tm_year = 70;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "FRI JAN 01 00:00:00 1971\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = -24;
    tmptr->tm_hour = 5;
    tmptr->tm_mday = 1;
    tmptr->tm_mon = 0;
    tmptr->tm_year = 70;
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "THU JAN 01 04:36:00 1970\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 30;
    tmptr->tm_mon = 1; 
    tmptr->tm_year = 74; 
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "SAT MAR 02 00:00:00 1974\n") == 0);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;

/* @@@ ldt
    mktime appears to fail when it is a leap year and 30 days is specified for
    Feburary in a leap year. 

    tmptr->tm_mday = 30;
*/
    tmptr->tm_mday = 30;
    tmptr->tm_mon = 1; 
    tmptr->tm_year = 72; 
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "WED MAR 01 00:00:00 1972\n") == 0);
    printf ("buffer = %s", buffer);

    tmptr->tm_sec = 0;
    tmptr->tm_min = 0;
    tmptr->tm_hour = 0;
    tmptr->tm_mday = 32;
    tmptr->tm_mon = 0; 
    tmptr->tm_year = 70; 
    restime = mktime(tmptr);
    buffer = ctime(&restime);
    result += ASSERT(strcmp(buffer, "SUN FEB 01 00:00:00 1970\n") == 0);

	if (result == 0)
		fdprintf(2, "SUCCESS testing mktime\n");
	   else
		fdprintf(2, "Failed %d times testing asctime\n", result);
	return(result);
    }

