/* echoClient.c - TCP echo client program */

/* Copyright 1984-1997 Wind River Systems, Inc. */
 
/*
modification history
--------------------
01f,06nov97,mm   added copyright.
01e,29Sep97,mm   cast arg 1 of `bzero' , cast arg 2 of `connect', cast arg 4 of `setsockopt'
01d,17Sep97,mm   deleted line IMPORT int tcp_keepidle = 1
01c,17Sep97,mm   changed int optionVal to char optionVal 
01b,16Sep97,mm   included <stdio.h> <string.h> <arpa/inet.h>
01a,07Feb94,ms   cleaned up for VxDemo.
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <vxWorks.h>
#include "socket.h"
#include "in.h"
#include "ioLib.h"
#include "errno.h"
#include "netinet/tcp.h"
#include <sockLib.h>

LOCAL int  defaultTimes	= 50;   /* default number of times for echo service */
LOCAL int  defaultPort	= 7001; /* default port number used by echoTcpServer */
LOCAL char  defaultServer[16] = "147.11.184.2";  /* default echo server's inet 
					      * address */
LOCAL char defaultMessage[80] = "Hello World!!!\n"; /* Buffer for the default 
						 * message */

/* Change the default keepidle time (timeout for probing the idle
* connection) from several hours to few minutes.
*/
/*IMPORT int tcp_keepidle = 1;*/

LOCAL int echoTcpClientSock (char *, int); /* set up TCP socket */
LOCAL void  echoTcpClient (int, int, char*); /*UDP echo client*/

/*****************************************************************************
 * echoTcpClientRun - Runs the TCP client task for echo service
 *
 * DESCRIPTION
 *
 *     The TCP echo client repeatedly sends the input and then reads it back 
 *     using TCP socket communication.
 *
 *     ECHO services are important tools that network managers use to test
 *     reachability (to make sure the connection works), debug protocol 
 *     software, and identify routing problems.
 *
 *     This also demonstrates the usage of SO_REUSEADDR, SO_KEEPALIVE and
 *     TCP_NODELAY set socket options.
 * 
 * CONFIGURATION: 
 *
 * You need to set/change the value of the defaultPort (should be greater than
 * 5000 - unreserved port) and the defaultServer (internet address of the
 * echo server) variables.
 *
 * EXAMPLE:
 *
 *     Run this echoTcpClientRun task on one VxWorks system as follows
 *     after starting the echoTcpServerRun task from another VxWorks system.
 *     The other half of the demonstration is in echoServer.c.
 *
 *     sp (echoTcpClientRun, inetAddrOfServer, port, numTimes, msg)
 *
 *     where inetAddrOfServer is the internet address of the network
 *     interface of the VxWorks system running echoTcpServerRun task,
 *     port is the port number used by the echoTcpServer (should be
 *     greater than 5000), numTimes is the number of times echo service is
 *     requested, and msg is the message to echo.
 *     
 *     example:
 *     -> sp (echoTcpClientRun, "147.11.184.2", 7001, 5, "Hello World!!!")
 *
 */

void echoTcpClientRun
    (
    char *server,                /* internet address of the echo server; 
                                  * 0 to use default */ 
    int  port,                   /* port number; 0 to use the last used one*/
    int  numTimes,               /* number of times for echo service */
    char *msg                    /* Message to echo; 0 to use default or last
                                    used message */ 
    )
    {
        int clientSock;          /* socket fd opened to the server */
	clientSock = echoTcpClientSock (server, port);
        if (clientSock == ERROR)
            printf ("Error creating client socket\n");
        else
 	    echoTcpClient (clientSock, numTimes, msg);
        close (clientSock);
    }

/*****************************************************************************
 * echoTcpClientSock - Sets up a TCP socket connection with echo client 
 *
 * DESCRIPTION
 *
 *     Sets up a TCP socket connection with echo server with various socket
 *     options.
 *
 * RETURNS: OK or ERROR
 */


int echoTcpClientSock 
    (
    char 	*server,   /* inet number of echo server */
    int 	port       /* port number */
    )
    {
    int			echoClientSock; /* socket opened to echo server */
    int			optionVal;      /* value of setsocket option */
    struct sockaddr_in	serverAddr;    /* server's address */


    if (server == NULL)
	server = defaultServer;

    if (port == 0)
	port = defaultPort;
    else
	defaultPort = port;

    if (port < 5000)
        {
        printf ("echoTcpClientSock: invalid port number\n");
        return (ERROR);
        }

    /* open the socket */
    echoClientSock = socket (AF_INET, SOCK_STREAM, 0);

    if (echoClientSock == ERROR)
	{
	perror ("echoTcpClientSock: socket open failed");
	return (ERROR);
	}

    optionVal = 1; /* Turn ON the different setsockopt options */

    /* Specify the SO_REUSEADDR option  to bind a stream socket to a local  
     * port  that may be still bound to another stream socket that may be 
     * hanging around with a "zombie" protocol control block whose context
     * is  not yet freed from previous sessions.
     */
    if (setsockopt (echoClientSock, SOL_SOCKET, SO_REUSEADDR, (char *)&optionVal,
		    sizeof(optionVal)) == ERROR)
	{
	perror ("echoTcpClientSock: setsockopt SO_REUSEADDR failed");
	close (echoClientSock);
	return (ERROR);
	}

    /* Specify the TCP_NODELAY option for protocols such as X Window System 
     * Protocol that require immediate delivery of many small messages. 
     *
     * By default VxWorks uses congestion avoidance algorithm 
     * for virtual  terminal  protocols and  bulk  data  transfer  
     * protocols. When the TCP_NODELAY option is turned on and there are 
     * segments to be sent out, TCP  bypasses  the  congestion
     * avoidance algorithm  and sends the segments out when there 
     * is enough space in the send window.
     */

    if (setsockopt (echoClientSock, IPPROTO_TCP, TCP_NODELAY, (char *)&optionVal,
		    sizeof(optionVal)) == ERROR)
	{
	perror ("echoTcpClientSock: setsockopt TCP_NODELAY failed");
	close (echoClientSock);
	return (ERROR);
	}

    /* Specify the SO_KEEPALIVE option, and the transport protocol (TCP)  
     * initiates  a timer to detect a dead connection which prevents an
     * an application from  hanging on an invalid connection.
     */
    if (setsockopt (echoClientSock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optionVal,
		    sizeof(optionVal)) == ERROR)
       {
       perror ("echoTcpClientSock: setsockopt SO_KEEPALIVE failed");
       close (echoClientSock);
       return (ERROR);
       }
   
    /* Set up server's internet address and connect to the server */

    /* Zero out the sock_addr structures.
     * This MUST be done before the socket calls.
     */
    bzero ((char  *) &serverAddr, sizeof (serverAddr));
    serverAddr.sin_family	= AF_INET;
    serverAddr.sin_port	= htons (port);
    serverAddr.sin_addr.s_addr	= inet_addr (server);

    printf ("Server's address is %x:\n", ntohl (serverAddr.sin_addr.s_addr));
    if (connect (echoClientSock, (struct sockaddr  *) &serverAddr, sizeof (serverAddr)) < 0)
	{
	perror ("echoTcpClientSock: connect failed");
	close (echoClientSock);
	return (ERROR);
	}
    printf ("Connected...\n"); 

    return (echoClientSock);
    }

/*****************************************************************************
 * echoTcpClient - the TCP client for echo service
 *
 * DESCRIPTION
 *
 *     echoTcpClient enters a loop that repeatedly sends message across the
 *     TCP connection to the echo server, reads it back and prints it.
 *     After all input messages have been sent to the server, received back,
 *     and printed successfully, the client exits.  
 *
 */

void echoTcpClient 
    (
    int echoClientSock,           /* client socket fd */
    int   times,                  /* number of times for echo service */
    char *message                 /* message buffer for echo service */
    )
    {
    int		ix;               /* loop counter */
    int		msgLen;           /* length of the message */
    char 	buffer [80];      /* echoed message buffer */
    int 	numToRead;        /* number of bytes to read */ 
    int 	numRead;          /* number of bytes read */

    if (times == 0)
	times = defaultTimes;
    else
	defaultTimes = times;

    if (message == NULL)
	message = defaultMessage;
    
    msgLen = strlen (message);
    printf ("The length of the message is - %d bytes\n", msgLen); 
    printf ("Message: %s\n\n", message);
    for (ix = 0; ix < times; ++ix)
	{

	if (send (echoClientSock, message, msgLen, 0) < 0)
            {
            perror ("echoTcpClient: echo CLIENT write error");
            return;
            }

        /* number of bytes to be read is assumed to be less than 80
         * bytes and the message is NULL terminated for this demonstration.
         */
	for (numToRead = msgLen; numToRead > 0; numToRead -= numRead)
	    {
	    buffer[0] = '\0';
	    numRead = recv (echoClientSock, buffer, numToRead, 0);
	    printf ("echoTcpClient received echo message: %s\n", buffer);
	    if (numRead == ERROR)
		{
                perror ("echoTcpClient: echo CLIENT recv error");
                break;
		}
	    }
	}
    }
