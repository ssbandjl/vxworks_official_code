/* broadcastsend.c - Demo for sending a broadcast  message */

/*
modification history
--------------------
 
01f,25nov97,rv   added return TRUE 
01e,22sep97,mm   defined  broadcastSend(int port)
01d,22sep97,mm   cast arg 4 of setsockopt function
01c,22sep97,mm   cast arg 5 of sendto function
01b,22sep97,mm   added include <string.h> <errno.h> <unistd.h> <arpa/inet.h> 
01a,08feb94,ms   cleaned and modified for VxDemo.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "vxWorks.h"
#include "socket.h"
#include "sockLib.h"
#include "in.h"
#include <arpa/inet.h>

#define BROADCAST_ADDR "192.192.192.255" /* your subnet network's broadcast 
                                          * inet address
                                          */

LOCAL char buf[] = "Hello, world!!!\n";  /* broadcast message */

/*****************************************************************************
 * broadcastSend - Sends broadcasting message 
 *
 * DESCRIPTION
 *
 *     Demo for sending broadcasting message.
 *
 * RETURNS: OK or ERROR
 *
 * CONFIGURATION:
 *
 * You need to set/change the value of the BROADCAST_ADDR constant to 
 * your (subnet) network's broadcast inet address.
 *
 * EXAMPLE:
 *
 *     Run broadcastSend task on one VxWorks system as follows
 *     after starting the broadcastGet task from another VxWorks system
 *     in the same physical network. The other half of the demonstration
 *     is in broadcastGet.c.
 *
 *     -> sp (broadcastSend,  7001)
 *
 *     where 7001 (port number should be greater than 5000 for user-developed)
 *     is an example port number used in this demonstration to send the
 *     broadcast message.
 *
 */

STATUS broadcastSend(int port);

STATUS broadcastSend 
    (
    int port                             /* port number */
    )

    {
    int sockFd;                          /* socket fd */
    struct sockaddr_in sendToAddr;       /* receiver's addresss */
    int   sendNum ;
    int   on;


    /* Open UDP socket */
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd == ERROR)
   	{ 
	perror ("socket not opened ");
	return (ERROR);
	}


    /* Use the SO_BROADCAST option when an application needs to broadcast data
     */ 
 
    on = 1; /* turn ON SO_BROADCAST option*/
    if (setsockopt (sockFd, SOL_SOCKET, SO_BROADCAST, (char *) &on, sizeof(int))  == 
                     ERROR)
	{
	perror ("setsockopt failed ");
	return (ERROR);
	}

    /* zero out the sockaddr_in structures and setup receivers' address */ 
    bzero ((char *) &sendToAddr, sizeof (struct sockaddr_in));
    sendToAddr.sin_family = AF_INET;
    sendToAddr.sin_port = htons (port);
    sendToAddr.sin_addr.s_addr = inet_addr (BROADCAST_ADDR);

    /* send the broadcast message to other systems in the same network */
    if ((sendNum = sendto (sockFd, buf, sizeof (buf), 0, (struct sockaddr  *) &sendToAddr, 
	sizeof (struct sockaddr_in))) == ERROR)
    	{
    	perror ("sendto broadcast failed ");
       	return (ERROR);
	}

    printf ("%d bytes of broadcast message sent: %s\n", sendNum,
	            buf);
    close (sockFd);
    return(TRUE);
    }
