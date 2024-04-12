/* broadcastGet.c - demo for getting a broadcast message */
 
/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01d,15Sep97,mm   cast arg 2 of bind
01c,15Sep97,mm   defined function broadcastGet
01b,15Sep97,mm   changed "if.h" to "net/if.h", added stdio.h, unistd.h, 
                 and sockLib.h
01a,08Feb94,ms   cleaned and modified for VxDemo.
*/

#include "vxWorks.h"
#include "types.h"
#include "string.h"
#include "socket.h"
#include "sockLib.h"
#include "stdio.h"
#include "in.h"
#include "net/if.h"
#include "ioctl.h"
#include "unistd.h"

#define BUFSIZ (sizeof (struct ifreq) * 4)/*size of buffer for SIOCFGIFCONFIG*/

/*****************************************************************************
 * broadcastGet - Get the broadcast message 
 *
 * DESCRIPTION
 *
 *     Demo for getting the broadcast message. Also demonstrates the usage
 *     of SIOCGIFCONF ioctl option. This SIOCGIFCONF ioctl option is not needed
 *     for broadcasting.  
 *
 * RETURNS: OK or ERROR
 *
 * EXAMPLE:
 *
 *     Run broadcastGet task on one VxWorks system as follows
 *     before starting the broadcastSend task from another VxWorks system
 *     in the same physical network. The other half of the demonstration
 *     is in broadcastSend.c.
 *
 *     -> sp (broadcastGet,  7001) 
 *
 *     where 7001 (port number should be greater than 5000 for user-developed)
 *     is an example port number used in this demonstration to receive the
 *     broadcast message.
 *    
 *
 */

STATUS broadcastGet(int port);

STATUS broadcastGet 
    (
    int port                                          /* port number */      
    )
    {
    int sockFd;                   /* socket fd */                   
    struct sockaddr_in sockAddr;  /* socket address to recv from */ 
    char  buf[BUFSIZ];           /* buffer space to hold SIOCGIFCONFIG option*/
    char message[50];             /* buffer for broadcast message */
    int   recvNum;                /* number of bytes received */ 
    struct ifconf ifc;           
    struct ifreq  *ifr;

    /* open UDP socket */
    sockFd = socket (AF_INET, SOCK_DGRAM, 0);
    if (sockFd == ERROR)
   	{ 
	perror ("socket not opened \n");
	return (ERROR);
	}

    /* Demonstrates the usage of SIOCGIFCONF ioctl option. 
     * This SIOCGIFCONF ioctl option is not really needed for broadcasting.  
     *
     * Following code segment demonstrates the use of SIOCGIFCONF ioctl option.
     * Here SIOCGIFCONF option is used to obtain the name of the network
     * interface. This code segment is not required for receiving broadcasting 
     * message.
     */

    ifc.ifc_len = sizeof (buf); 
    ifc.ifc_buf = buf;
    if (ioctl (sockFd, SIOCGIFCONF, (char *)&ifc) == ERROR) 
	{
	perror ("broadcast: ioctl (get interface configuration)");
     	return (ERROR);
        }

    ifr = ifc.ifc_req;
    /* Print the name of the underlying network interface using SIOCGIFCONF
     * ioctl option.
     */
    printf ("SIOCGIFCONF: This socket uses the network interface %s\n\n", 
             ifr->ifr_name);

        

    /* Zero out and  fill in sockaddr_in structure to receive from*/
    bzero ((char *) &sockAddr, sizeof (struct sockaddr_in));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons (port);
    sockAddr.sin_addr.s_addr = INADDR_ANY; 

    /* bind to the socket */ 
    if (bind (sockFd, (struct sockaddr *)& sockAddr, sizeof (sockAddr)) == ERROR)
	{
	perror ("bind failed");
	return (ERROR);
	}

    FOREVER
        /* receive the broadcast message */
        if ((recvNum = recv (sockFd, message, sizeof (message), 0)) == ERROR)
            {
            perror("recv broadcast failed ");
            break;
	    }
        else
            printf ("received %d bytes of broadcast message:  %s\n", recvNum, message);
    close (sockFd);
    }
