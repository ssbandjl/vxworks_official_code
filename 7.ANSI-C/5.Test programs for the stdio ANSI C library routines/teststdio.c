/* To test the stdio ANSI C library */

/* Copyright 1991 Wind River Systems, Inc. */

/*
modification history
--------------------
02a,01aug97,ram		modified and documented.
01a,02apr92,smb  
*/

/*
DESCRIPTION

The following functions fopen, fclose and freopen have been tested here.
fopen() - This is used to open a file. fopen returns a pointer to the 
beginning of the buffer area associated with the file or a NULL 
if if the file cannot be opened.
fclose() - A data file must always be closed at the end of a program
and this is achieved using the fclose function.
freopen() - This is used to attach preopened streams to other files.

INCLUDE FILES: test.h
*/

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "errno.h"
#include "float.h"
#include "math.h"
#include "stdarg.h"
#include "string.h"
#include "test.h"

/***************************************************************************
*
* teststdio - To test the opening and closing of files.
*
* RETURNS: OK on success else gives an ASSERTION ERROR
*/

int teststdio()
	{
	char buffer[10];
	char *tn = "tmpxxx";
	FILE *pf = NULL;
	int in1, result = 0;
	fpos_t fp1;
	long off;

/* opening a file for both reading and writing */
	result += ASSERT((pf = fopen(tn, "w+")) != NULL);
	
/* closing the file  */
	result += ASSERT(fclose(pf) == 0);

/* reopening the file and attaching the standard input stream to the file */
	result += ASSERT(freopen(tn, "r", stdin) == stdin);

/* closing the file  */
	result += ASSERT(fclose(stdin) == 0);

/* opening the file for updating */
	result += ASSERT((pf = fopen(tn, "w+b")) != NULL);

/* closing the file  */
 	fclose(pf);
	
	if(result==0)
	{
	 printf("Success testing stdio \n");
	}
	else
	{
	printf("ERROR \n");
	}
	return(result);
	}
