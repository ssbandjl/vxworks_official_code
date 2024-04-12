/* printEtherAddrs.c - Prints the ethernet address of a network
 *                     interface on the VxWorks target
 */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm   added copyright.
01b,15Sep97,mm   included <string.h> and changed "if.h" to "net/if.h"
01a,16Feb93,ms   written
*/

#include <string.h>
#include "vxWorks.h"
#include "stdio.h"
#include "net/if.h"
#include "etherLib.h"

/*****************************************************************************
 * printEtherAddrs - Demo for printing the ethernet address of the network
 *                   interface in the VxWorks target.
 *
 * EXAMPLE:
 *
 *     To run printEtherAddrs from the VxWorks shell do as follows: 
 *
 *     ->  printEtherAddrs ("nameOfTheNetworkInterface")
 *
 *     where nameOfTheNetworkInterface is the name of the network
 *     interface in your the VxWorks target (Eg: "ln0")
 *
 */
   
void printEtherAddrs 
    (
    char *netIfName        /* name of the network interface */
    )
    {
    struct ifnet    *ifPtr; /* Pointer to network interface's ifnet struct */
    unsigned char enet[6];  /* This is a temporary variable used in this demo
                             * for storing the Ethernet address of the network
                             * interface.
                             */
                            

    if ((ifPtr = ifunit (netIfName)) == NULL)
        printf ("Interface not found \n");
    else
        {       
        /* get the ethernet address from the arpcom structure (arpcom
         * structure is shared between the network interface 
         * driver and address resolution code) and print it. The first data
         * element in an arpcom structure (vw/h/netinet/if_ether.h) is an 
         * ifnet structure (vw/h/net/if.h) (hence the address of arpcom and 
         * ifnet structures are the same for any given network). 
         */
        bcopy ((char *) ((struct arpcom *) ifPtr)->ac_enaddr, enet, 
                sizeof (enet)); 
        printf(" Ethernet address is %02x:%02x:%02x:%02x:%02x:%02x\n",
                  enet [0], enet [1], enet [2], enet [3], enet [4], enet[5]);
	}
    }
