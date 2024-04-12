/* vxServer.c - Example code of UDP server. */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01d,06nov97,mm   added copyright.
01c,26Sep97,mm   cast agr.1 of bzero
01b,26Sep97,mm   added include <stdio.h> , <string.h> , <unistd.h>
01a,09Feb94,ms   cleaned up for VxDemo.
*/

#include "vxWorks.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "in.h"
#include "inetLib.h"
#include "udpExample.h" 

/*****************************************************************************
 * myServer - Server reads from local socket and displays client's message. 
 * If requested, server sends a reply back to the client. Code executes on
 * VxWorks target and use VxWorks header files.
 *
 *  EXAMPLE:
 *
 *     To run this myServer task, from the VxWorks shell do as follows:
 *     -> sp (vxUdpServer)
 *
 *  RETURNS: OK or ERROR
 */
 
STATUS vxUdpServer()
    {
    struct sockaddr_in myAddr;		/* Server socket address */
    struct sockaddr_in clientAddr;	/* Socket address for client */
    struct request clientRequest;	/* Request/Message from client */
    int sFd;				/* Server's socket file descriptor */
    char inetAddr[INET_ADDR_LEN];	/* Buffer for dot notation *
                        		 * internet addr of client */

    /* Size of socket address structure */
    int sockAddrSize = sizeof (struct sockaddr_in);

    LOCAL char replyMsg[] = "Server received your message";

    /* Build socket address */
    bzero ((char *) &myAddr, sockAddrSize);
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons (SERVER_PORT_NUM);
    myAddr.sin_addr.s_addr = htonl (INADDR_ANY);

    /* Create socket */
    if ((sFd = socket (AF_INET, SOCK_DGRAM, 0)) == ERROR)
        {
	perror ("socket");
	close (sFd);
	return (ERROR);
	}

    /* Bind socket to local address */
    if (bind (sFd, (struct sockaddr *) &myAddr, sockAddrSize) 
                                                       	== ERROR)
        {
        perror ("bind");
	close (sFd);
	return (ERROR);
	}


    FOREVER
        {
	/* Read data from a socket and satisfy requests */
	if (recvfrom (sFd, &clientRequest, sizeof (clientRequest), 0,
		      (struct sockaddr *) &clientAddr,&sockAddrSize) == ERROR)
            {
            perror ("recvfrom");
            close (sFd);
            return (ERROR);
            }
        /* Convert internet address to dot notation for displaying */
	inet_ntoa_b (clientAddr.sin_addr, inetAddr);
	printf ("MESSAGE FROM: Internet Address %s, port %d\n%s\n",
 		inetAddr, ntohs (clientAddr.sin_port), clientRequest.message);

	/* If client requested a reply, send acknowledgment */
	if (clientRequest.reply)
            if (sendto (sFd, replyMsg, sizeof (replyMsg), 0, 
                        (struct sockaddr *) &clientAddr,sockAddrSize) == ERROR)
            {
            perror ("sendto");
            close (sFd);
            return (ERROR);
            }
        }

    close (sFd); /* Just in case. Should never get here. */
    }
