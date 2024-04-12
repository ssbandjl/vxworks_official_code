/* unixCient.c - Example code of UDP client */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm   added copyright.
01b,24Sep97,mm   cast arg 5 of sendto and recvfrom   
01a,10Feb94,ms   cleaned up for VxDemo.
*/


/***********************************************************************
 * DESCRIPTION
 *     Code to be executed by client. Writes request/message nto socket
 *     and optionally receives a reply back. Code executes on UNIX,
 *     therefore UNIX header files are included.
 *
 * 
 * EXAMPLE
 *     On the UNIX host:
 *     % unixClient 
 *
 */
	

/* Code to be executed by client. Writes request/message
 * into socket and optionally receives a reply back.
 */

	

/* Code executes on UNIX therefore include UNIX header files. */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "udpExample.h"
#include "client.h"


main()

    {
    int sFd;				/* Socket file descriptor */
    int mlen;				/* Length of message to send */
    struct request myRequest;		/* Request/Message to send to server */
    struct sockaddr_in replyAddr;	/* If expecting reply,
 					 * address of node that sent
					 * the reply */
    char reply;				/* TRUE if want reply */
    char replyBuf[REPLY_MSG_SIZE];	/* Buffer to store reply
					 * from server. */
    struct sockaddr_in serverSockAddr; /* Address of server */

    /* Size of socket address structure */
    int sockAddrSize = sizeof (struct sockaddr_in);

    /* Create socket */
    if ((sFd = socket (AF_INET, SOCK_DGRAM, 0)) == ERROR)
        {
	fprintf (stderr, "Error creating socket\n");
	return (ERROR);
	}

    /* Bind not required - port number will be dynamic */
    /* Build server socket address */
    bzero (&serverSockAddr, sockAddrSize);
    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_port = htons (SERVER_PORT_NUM);
    serverSockAddr.sin_addr.s_addr = inet_addr (SERVER_INET_ADDR);

    /* Build request/message , prompting user */
    printf ("Message to send: \n");
    mlen = read (STDIN_FILENO, myRequest.message, MSG_SIZE);
    myRequest.message[mlen - 1] = '\0'; 
    printf ("Would you like a reply (Y or N): \n");
    scanf ("%c",&reply);

    switch (reply)
        {
	case 'y':
	case 'Y': myRequest.reply = TRUE;
 	    break;

	default: myRequest.reply = FALSE;
            break;
	}

    /* Send message/request to server */
    if (sendto (sFd, (caddr_t) &myRequest, sizeof (myRequest), 0,
                  (struct sockaddr  *) &serverSockAddr,sockAddrSize) == ERROR)
        {
	perror ("sendto");
	close (sFd);
	return (ERROR);
	}

    /* If expecting reply, read and print it */
    if (myRequest.reply)
        {
	/* Should use select here. If request or reply get lost,
  	 * this code will block forever. */

        if (recvfrom (sFd, replyBuf, REPLY_MSG_SIZE, 0,
            (struct sockaddr  *) &replyAddr, &sockAddrSize) == ERROR)
            {
	    perror ("recvfrom");
	    close (sFd);
	    return (ERROR);
	    }

        printf ("MESSAGE FROM SERVER:\n%s\n", replyBuf);
        }

    close (sFd);

    }

