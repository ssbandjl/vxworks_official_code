/*  teststdio2.c - To test the stdio ANSI C library */

/* Copyright 1991 Wind River Systems, Inc. */

/*
modification history
--------------------
02b,31jul97,ram		need to add INCLUDE_DOSFS & INCLUDE_RAMDRV in config.h
			for successful running.
			added ramDrv.h header file,documented. 

01a,02apr92,smb  
*/

/*
INCLUDE FILES:	test.h
*/

/* includes */ 

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "float.h"
#include "math.h"
#include "stdarg.h"
#include "dosFsLib.h"
#include "string.h"
#include "ramDrv.h"
#include "test.h"

/* locals */

int stdioT2On  = 10;
int stdioFsDev = FALSE;
		
/***************************************************************************
*
* teststdio2 - Test the stdio ANSI C library
*
* This routine tests the various types (basically 3 i.e, stdin, stdout and
* stderr), macros and functions for performing input and output as 
* declared by stdio.h
*
* RETURNS: OK on success, or an ASSERTION ERROR if any of the functions fail
*/

int teststdio2()
	{
	int result = 0;
	char buf[32]; /* used as a buffer to store data */

	FILE *pf;      /* pf is a pointer to the beginning of a buffer   */
		       /* area established by the special structure FILE */

	char filename1[64]; /* variable to store file name */
	char filename2[64]; /* variable to store file name */

/* macros delcared by stdio.h */

	static int macs[] = {
			    _IOFBF, _IOLBF, _IONBF, BUFSIZ, EOF, FILENAME_MAX,
			    FOPEN_MAX, TMP_MAX, SEEK_CUR, SEEK_END, SEEK_SET};

	printf("MACROS AS DECLARED IN <stdio.h> HEADER FILE: \n");

	   printf("_IOFBF 	= %d \n",macs[0]);
	   printf("_IOLBF 	= %d \n",macs[1]);
	   printf("_IONBF 	= %d \n",macs[2]);
	   printf("BUFSIZ 	= %d \n",macs[3]);
	   printf("EOF    	= %d \n",macs[4]);
	   printf("FILENAME_MAX = %d \n",macs[5]);
	   printf("FOPEN_MAX    = %d \n",macs[6]);
	   printf("TMP_MAX      = %d \n",macs[7]);
	   printf("SEEK_CUR     = %d \n",macs[8]);
	   printf("SEEK_END     = %d \n",macs[9]);
	   printf("SEEK_SET     = %d \n",macs[10]);

	result += ASSERT(256 <= BUFSIZ && EOF < 0);
	result += ASSERT(8 <= FOPEN_MAX && 25 <= TMP_MAX);

	if (stdioFsDev != 1)
	    {

/*
* dosFsMkfs - provides a quick method of creating and 
*             initializing a dosFsMkfs file system on a device. 
* ramDevCreate - creates a ram disk device	
*/

 	    printf("creating a ram disk of size 200k ... \n");
	    dosFsMkfs ("dev1:", ramDevCreate (0,512,400,400,0)); 
	    printf("check \n");
	    stdioFsDev = TRUE;
	    }

	while ((stdioT2On--) >= 0)
	    {
	    strcpy (filename1, "dev1:/");
/*
* tmpnam - This function generates a file name that can be safely used
*          for a temporary file. If NULL is used as the parameter then
*	   tmpnam leaves its result in an internal static area and
*	   returns a pointer to that area.
*/
	    strcat (filename1, tmpnam(NULL));
	    strcpy (filename2, "dev1:/");
	    strcat (filename2, tmpnam(NULL));
	     
            printf("Opening file: %s for writing\n",filename1);
	    if ((pf = fopen(filename1, "w")) == NULL)
		{
		printf (" ERROR \n");
		return (ERROR);
		}
	    else
	        {
	        printf("successfully opened file: %s\n",filename1);
		}
	    
/*
* Testing the various functions declared by stdio.h
* Each functions returns a null/OK on success. 
* ASSERT- returns null if assertion/condition is true
* 	  else returns an error message of type
* "  Assertion failed: expression, file xyz, line nnn  "
* If every function returns OK the value of result remains null.
*/


	    result += ASSERT(pf!=NULL && pf!=stdin && pf!=stdout && pf!=stderr);
	    result += ASSERT(feof(pf) == 0 && ferror(pf) == 0);
	    if( result == 0 )
	        {
		printf("feof() tested. \n");
		}

	    result += ASSERT(fgetc(pf) == EOF);
	    if( result == 0 )
	        {
		printf("fgetc() tested. \n");
		}

	    result += ASSERT(feof(pf) == 0 );
	    result += ASSERT(ferror(pf) == 0); /* when does a read error occur*/
	    if( result == 0 )
	        {
		printf("ferror() tested. \n");
		}

	    clearerr(pf); /* clears end-of-file & error flags for a stream */

	    result += ASSERT(ferror(pf) == 0);
	    result += ASSERT(putc('a', pf) == 'a');
	    if( result == 0 )
	        {
		printf("putc() tested. \n");
		}

	    result += ASSERT(fputc('b', pf) == 'b');
	    if( result == 0 )
	        {
		printf("fputc() tested. \n");
		}

	    result += ASSERT(0 <= fputs("cde\n", pf));
	    if( result == 0 )
	        {
		printf("fputs() tested. \n");
		}

	    result += ASSERT(0 <= fputs("fghij\n", pf));
	    result += ASSERT(fflush(pf) == 0);
	    if( result == 0 )
	        {
		printf("fflush() tested. \n");
		}

	    result += ASSERT(fwrite("klmnopq\n", 2, 4, pf) == 4 );
	    if( result == 0 )
	        {
		printf("fwrite() tested. \n");
		}

	    result += ASSERT((fclose(pf)) == 0 );
	    if( result == 0 )
	        {
		printf("fclose() tested. \n");
		}

	    result += ASSERT((freopen(filename1, "r", stdin)) == stdin);
	    if( result == 0 )
	        {
		printf("freopen() tested. \n");
		}

	    result += ASSERT(fgetc(stdin) == 'a' && getc(stdin) == 'b');
	    if( result == 0 )
	        {
		printf("fget() tested. \n");
		}

	    result += ASSERT(getchar() == 'c');
	    if( result == 0 )
	        {
		printf("getchar() tested. \n");
		}

	    result += ASSERT(fgets(buf, sizeof(buf), stdin) == buf &&
			     strcmp(buf, "de\n") == 0);
	    if( result == 0 )
	        {
		printf("fgets() tested. \n");
		}

	    result += ASSERT(ungetc('x', stdin) == 'x');
	    if( result == 0 )
	        {
		printf("ungetc() tested. \n");
		}

	    result += ASSERT(gets(buf) == buf && strcmp(buf, "xfghij") == 0);
	    if( result == 0 )
	        {
		printf("gets() tested. \n");
		}

	    result += ASSERT(fread(buf, 2, 4, stdin) == 4 &&
			     strncmp(buf, "klmnopq\n", 8) == 0);
	    if( result == 0 )
	        {
		printf("fread() tested. \n");
		}
	    fclose(pf);
	    result += ASSERT(getchar() == EOF && feof(stdin) != 0);

	    result += ASSERT(rename(filename1,filename2) == 0);
	    result += ASSERT(remove(filename2) == 0);
	    }

	if (result == 0)
		fdprintf(2, "SUCCESS testing stdio part 2 \n");
	   else
		fdprintf(2, "Failed %d times testing stdio part 2 \n", result);
	return(result);

	}
