
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

/* DEFINES */

#define BUFFERSIZE 100

/* 
DESCRIPTION
strftime: This function will  place  bytes  into  the
array pointed to by s as controlled by the string pointed to
by format.  The format  string  consists  of  zero  or  more
conversion   specifications   and  ordinary  characters.   A
conversion specification consists of a '%' (percent) charac-
ter  and  one  or two terminating conversion characters that
determine  the  conversion  specification's  behavior.   All
ordinary  characters  (including  the terminating null byte)
are copied unchanged into the array pointed  to  by  s.   If
copying  takes  place  between  objects  that  overlap,  the
behavior is undefined.  For strftime(), no more than maxsize
bytes are placed into the array.
RETURNS the number of characters placed into the array pointed
to by s, not including the terminating null character.  If the
total number of resulting characters including the terminating
null characters is more than maxsize, strftime() returns 0 and
the array are indeterminate.

assert:  If an expression is false (that is, equal to zero), the assert()
macro writes information about the failed call to standard erro 
in an implementation-defined format.  It then calls abort().  The
diagnostic information includes:

  -the test of the argument
  -the name of the source file (value of preprocessor macro __FILE__)
  -the source line number (value of preprocessor macro __LINE__).
RETURNS N/A.

strcmp: This routine compares two strings according to the
ordering  of  your  machine's  character  set.  The function
returns an integer greater than, equal to, or less  than  0,
if the string pointed to by s1 is greater than, equal to, or
less than the string pointed to by s2 respectively. The sign
of  a non-zero return value is determined by the sign of the
difference between the values of the  first  pair  of  bytes
that  differ in the strings being compared.  strncmp() makes
the same comparison but looks  at  a  maximum  of  n  bytes.
Bytes following a null byte are not compared.
RETURNS an integer greater than, equal to, or less than 0,
according to whether s1 is lexicographically greater than,
equal to, or less than s2, respectively.

fdprintf: This routine writes a formatted string to a 
specified file descriptor.  Its function and syntax
are toherwise identical to printf().
RETURNS the number of characters output, or ERROR if
there is an error during output.

*/
/* A test routine for strftime */
int testStrftime ()
    {
    char *s;
    struct tm *tmptr;
    size_t nchars;
    int result = 0;

    tmptr = (struct tm *) malloc(sizeof(*tmptr));
    s = (char *) malloc(BUFFERSIZE);

    tmptr->tm_sec = 59;
    tmptr->tm_min = 30;
    tmptr->tm_hour = 12;
    tmptr->tm_mday = 16;
    tmptr->tm_mon = 2;
    tmptr->tm_year = 92;
    tmptr->tm_wday = 1;
    tmptr->tm_yday = 99;

    nchars = strftime (s, 10, "%a", tmptr);
    result += ASSERT(strcmp(s,"MON") == 0);

    nchars = strftime (s, 10, "%A", tmptr);
    result += ASSERT(strcmp(s,"MONDAY") == 0);

    nchars = strftime (s, 10, "%b", tmptr);
    result += ASSERT(strcmp(s,"MAR") == 0);

    nchars = strftime (s, 10, "%B", tmptr);
    result += ASSERT(strcmp(s,"MARCH") == 0);

    nchars = strftime (s, 10, "%d", tmptr);
    result += ASSERT(strcmp(s,"16") == 0);

    nchars = strftime (s, 10, "%H", tmptr);
    result += ASSERT(strcmp(s,"12") == 0);

    nchars = strftime (s, 10, "%I", tmptr);
    result += ASSERT(strcmp(s,"00") == 0);

    nchars = strftime (s, 10, "%j", tmptr);
    result += ASSERT(strcmp(s,"100") == 0);

    nchars = strftime (s, 10, "%m", tmptr);
    result += ASSERT(strcmp(s,"03") == 0);

    nchars = strftime (s, 10, "%M", tmptr);
    result += ASSERT(strcmp(s,"30") == 0);

    nchars = strftime (s, 10, "%p", tmptr);
    result += ASSERT(strcmp(s,"AM") == 0);

    nchars = strftime (s, 10, "%S", tmptr);
    result += ASSERT(strcmp(s,"59") == 0);

    nchars = strftime (s, 10, "%U", tmptr);
    result += ASSERT(strcmp(s,"14") == 0);

    nchars = strftime (s, 10, "%w", tmptr);
    result += ASSERT(strcmp(s,"1") == 0);

    nchars = strftime (s, 10, "%W", tmptr);
    result += ASSERT(strcmp(s,"14") == 0);

    nchars = strftime (s, 10, "%y", tmptr);
    result += ASSERT(strcmp(s,"92") == 0);

    nchars = strftime (s, 10, "%Y", tmptr);
    result += ASSERT(strcmp(s,"1992") == 0);

    nchars = strftime (s, 10, "%Z", tmptr);
    printf ("s = %s\n", s); /* @@@ ldt */
    result += ASSERT(strcmp(s,"UTC") == 0);

    nchars = strftime (s, 10, "%%", tmptr);
    result += ASSERT(strcmp(s,"%") == 0);

    nchars = strftime (s, 10, "%aAB%ACDEFGH", tmptr);
    result += ASSERT(strcmp(s,"MONABMONDA") == 0);

    nchars = strftime (s, 25, "%a %b %d %H:%M:%S %Y\n", tmptr);
    result += ASSERT(strcmp(s,"MON MAR 16 12:30:59 1992\n") == 0);

    nchars = strftime (s, 5, "%A", tmptr);
    result += ASSERT(strcmp(s,"MONDA") == 0);

    nchars = strftime (s, 30, "%c", tmptr);
    result += ASSERT(strcmp(s,"MAR 16 12:30:59:1992") == 0);

    nchars = strftime (s, 30, "%X", tmptr);
    result += ASSERT(strcmp(s,"12:30:59") == 0);

    nchars = strftime (s, 30, "%x", tmptr);
    result += ASSERT(strcmp(s,"MAR 16 1992") == 0);

    nchars = strftime (s, 30, "", tmptr);
    result += ASSERT(strcmp(s,"") == 0);

	if (result == 0)
		fdprintf(2, "SUCCESS testing strftime\n");
	   else
		fdprintf(2, "Failed %d times testing strftime\n", result);
	return(result);
    }
