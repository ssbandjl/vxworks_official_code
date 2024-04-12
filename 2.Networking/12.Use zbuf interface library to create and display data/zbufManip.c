/* zbufManip.c - Zbuf structure manipulation */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/* 
modification history
--------------------
01b,11nov97,mm   added comments, merged display and manipulation routines. 
01a,24oct97,rks	 written 
*/

/*
DESCRIPTION
This module illustrates the use of zbufLib routines, and their effect on zbuf
segments and data sharing. To keep the example manageable, the zbuf data used
is artificially small, and the execution environment is the Tornado shell.
*/

/* includes */

#include "vxWorks.h"
#include "zbufLib.h"
#include "ioLib.h"
#include "stdio.h"
#include "string.h"

/* function prototype */

STATUS zbufDisplay
    (
    ZBUF_ID	zbufId, 
    ZBUF_SEG	zbufSeg, 
    int		offset,
    int		length, 
    BOOL	silent
    );

/********************************************************************
* zbufManip -
* 
* This routine illustrates the use of some zbuf routines and
* their effect on zbuf segements and data sharing.
*
* PARAMETERS: N/A
*
* RETURNS: N/N
*
*/

void zbufManip()
{
 ZBUF_ID	zId1;     	/* zbuf to manipulate and display */
 ZBUF_ID	zId2;     	/* zbuf to manipulate and display */

 char*		buff = "";   	/* data buffer */
 char* 		buff2 = "";   	/* data buffer */


 /* Create a zbuf */

 zId1 = zbufCreate();

 buff = "I cannot repeat enough";	  
 printf ("Inserting data from buff into the zbuf zId1.\n");
 printf ("Contents of buff -- I cannot repeat enough \n");

 zbufInsertBuf(zId1, 0, 0, buff, strlen(buff), 0, 0);


 /* Display contents of zbuf */

 printf ("Displaying contents of zId1:\n ");
 zbufDisplay(zId1,0,0,0,0);
 printf ("\n\n");

 /* Copy contents of zId1 to another zbuf zId2 */

 printf ("Copying contents of zId1 to zId2 \n");
 zId2 = zbufDup(zId1,0,0,23);
 zbufDisplay(zId2,0,0,0,0);
 printf ("\n\n");

 printf ("Now the copy has its own Id but uses the same address \n");
 printf ("Inserting a second buffer into the middle of the existing data \n");
 printf ("in zId1 gives us a zbuf made up of three segments. \n");
 printf ("Contents of buffer being entered is 'this' \n\n"); 
 buff2 = "this";       

 zbufInsertBuf(zId1, 0, 15, buff2, strlen(buff2), 0, 0);
 zbufDisplay(zId1,0,0,0,0);
 printf ("\n\n");

 printf ("Because the underlying buffer is not modified both buff and the\n");
 printf ("duplicate zId2 still contain the original String. \n");
 printf ("Contents of buff: ");
 printf ( buff );
 printf ("\n");
 printf ("Contents of zId2: ");
 zbufDisplay(zId2,0,0,0,0);
 printf ("\n");


 zbufDelete(zId1); /* Deleting the zbufs */
 zbufDelete(zId2);
}


/********************************************************************
* zbufDisplay 
*
* This routine displays the contents of a zbuf
*
* PARAMETERS:
*	      ZBUF_ID zbufId - zbuf to display
*             ZBUG_SEG zbufSeg - zbuf segment base for offset		
*	      int offset - relative byte offset
*	      int length - number of bytes to display
*	      BOOL silent - do not print out debug info
*
* RETURNS: OK, ERROR if the specified data could not be displayed.
*
*/

STATUS zbufDisplay
    (
    ZBUF_ID	zbufId, 
    ZBUF_SEG	zbufSeg, 
    int		offset,
    int 	length, 
    BOOL 	silent
    )
    {
    int		lenData;
    char *	pData;

    /* Find the most local byte location */

    if ((zbufSeg = zbufSegFind (zbufId, zbufSeg, &offset)) == NULL)
       return (ERROR);

    if(length <= 0)
       length = ZBUF_END;

    while((length != 0) && (zbufSeg != NULL))
        {
        /* Find location and data length of zbuf segment */

        pData = zbufSegData (zbufId,zbufSeg) + offset;
  	lenData = zbufSegLength (zbufId, zbufSeg) - offset;
  	lenData = min (length, lenData);

  	if(!silent)
   	  printf ("segID 0x%x at 0x%x + 0x%x (%2d bytes): ",
            (int) zbufSeg, (int) pData, offset, lenData);

        write (STD_OUT, pData, lenData);	/* Display data */
        if(!silent)
           printf ("\n");

        zbufSeg = zbufSegNext(zbufId, zbufSeg);    /* update segment */
        length -= lenData;    			   /* update length  */
        offset = 0; 				   /* no more offset */
        }

     return (OK);
     }


