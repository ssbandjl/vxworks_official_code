/* etherOutputDemo.c - Demo for using low-level output ethernet routines */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01d,06nov97,mm   added copyright.
01c,22Sep97,mm   added include string.h and "etherLib.h"
01b,22Sep97,mm   changed "if.h" to "netinet/if_ether.h"  
01a,16Feb94,ms   modified and cleaned up for VxDemo.
*/

#include "net/if.h"
#include "netinet/if_ether.h"
#include "etherDemo.h" 
#include <string.h> 
#include <stdio.h>  
#include "etherLib.h"  


/*****************************************************************************
 * etherOutputDemo - Demo for using low-level ethernet output routines
 *
 * DESCRIPTION
 *
 *     Sends (broadcast) data on an ethernet network interface 
 *     and handles (receives) raw input frames (input data) from the network
 *     interface using ether input hook routine etHandle.
 *
 *     This is a simple demonstration of using low-level output ethernet 
 *     routines. The other half of the demonstration of using low-level
 *     input ethernet routines is in etherInputDemo.c. etherInputDemo task  
 *     runs on one VxWorks system, and etherOutputDemo task runs on other 
 *     VxWorks system in the same physical network.
 *
 * RETURNS: OK or ERROR
 * 
 * EXAMPLE:
 *
 *     To run this etherOutputDemo task from the VxWorks shell do as follows: 
 *     -> sp (etherOutputDemo)
 *
 */

STATUS etherOutputDemo ()
    {
    if (etBcast (INTERFACE_NAME) == ERROR)
        { 
        printf ("etherOutput broadcast failed\n");
        return (ERROR);
        }
    else
        printf ("etherOutputDemo completed\n"); 
    return (OK);
    }


/*****************************************************************************
 * etBcast - Sends (broadcast) data on an ethernet network interface 
 *
 * RETURNS: OK or ERROR
 * 
 */

STATUS etBcast 
    (
    char *ifname       /* name of the network interface eg. "ei0" */
    )

    {
    struct ether_header ethHdr;   /* ethernet header */
    struct ifnet *pIfnet; /*Pointer to your network interface's ifnet struct*/

    pIfnet = ifunit (ifname); /* get the ifnet pointer of your network 
                               * interface 
                               */
    if (pIfnet == (struct ifnet *) NULL)
	{
	printf ("ifunit failed\n");
	return (ERROR);
	}

    /* Set the value of destination host's ethernet address -
       here broadcast ethernet address is set */
    bcopy (etbcastaddr, ethHdr.ether_dhost, sizeof (ethHdr.ether_dhost));
    ethHdr.ether_type = htons (ET_TYPE); /* user defined protocol */

    printf ("Protocol type 0x%x \n\n", (u_short)ethHdr.ether_type);
    /* send (broadcast) message on an ethernet interface */
    if (etherOutput (pIfnet, &ethHdr, etMessage, strlen (etMessage)) == ERROR)
        {
        perror ("etherOutput failed");
        return (ERROR);
        }
    else
        printf ("etherOutput done - message: %s \n", etMessage);
    return (OK);
    }

