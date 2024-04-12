/* etherInputDemo.c - Demo for using low-level ethernet input routines */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01e,06nov97,mm   added copyright.
01d,22Sep97,mm   removed include <vxWorks.h> changed include "etherDemo.h" to 
		 "in_etherDemo.h"
01c,22Sep97,mm   added include<etherLib.h> ,<string.h> ,<stdio.h> and <errno.h>
01b,22Sep97,mm   changed "if.h" to "net/if.h"
01a,16Feb94,ms   modified and cleaned up for VxDemo.
*/

#include <etherLib.h> 
#include <string.h>
#include <stdio.h>
#include <errno.h>  
#include "net/if.h"
#include "netinet/if_ether.h"
#include "in_etherDemo.h"

/*****************************************************************************
 * etherInputDemo - Demo for using low-level ethernet input routines
 *
 * DESCRIPTION
 *
 *     Handles (receives) raw input frames (input data) from the network
 *     interface using ether input hook routine etHandle.
 *
 *     This is a simple demonstration of using low-level input ethernet 
 *     routines. The other half of the demonstration of using low-level
 *     output ethernet routines is in etherOutputDemo.c. etherInputDemo runs on
 *     one VxWorks system, and etherOutputDemo runs on an other VxWorks system
 *     in the same physical network.
 *
 * RETURNS: OK or ERROR
 * 
 * EXAMPLE:
 *
 *     To run this etherInputDemo task from the VxWorks shell do as follows: 
 *     -> sp (etherInputDemo)
 *
 */

STATUS etherInputDemo ()
    {
    if (etherInputHookAdd (etHandle) == OK)
	printf ("etherInputHookAdd successful\n");
    else  
	{
        perror ("etherInputHook failed");
        return (ERROR);
        }

    return (OK);
    }


/*****************************************************************************
 * etHandle - ether input hook routine for handling raw input frames from the
 *            network interface. This routine is executed at task level 
 *            interrupt service for input packets.
 *
 * RETURNS: TRUE or FALSE
 * 
 */

BOOL etHandle (ifp, buffer, length)
    struct ifnet	*ifp;
    char		*buffer;
    int			length;

    {
    struct ether_header		*ethHdr;
    char 			*buffPtr;
    int 			ix;
    char                        tmp[25];

    ethHdr = (struct ether_header *) buffer;
    if (ntohs(ethHdr->ether_type) != (u_short) ET_TYPE)
	return (FALSE); /* Not a user defined protocol type, hence 0 is passed
                     * to indicate further processing of data to be handled by 
                     * higher level protocols (IP, TCP etc., ).
                     */


    /* Handle raw input frames. 
     * Data integrity check and printing the input message is done for 
     * demonstration purpose.
     */

    /* Check for data integrity error */
    buffPtr = (char *) (buffer + sizeof (*ethHdr));
    strncpy (tmp, buffPtr, strlen (etMessage));
    for (ix = 0; ix < sizeof (etMessage); ++ix)
	if (*buffPtr++ != etMessage [ix])
	    break;
    if (ix != strlen (etMessage))
        {
	logMsg ("etHandle: data integrity error at %d.\n", ix);
	return (ERROR);
	}

    logMsg ("etHandle: data okay - message is : %s \n", tmp);

    return (TRUE); /* non-zero value to imply hook routine has processed
                    * the input and there is no need for further handling by
                    * any higher level protocols (IP, TCP etc.) 
                    */
    }
