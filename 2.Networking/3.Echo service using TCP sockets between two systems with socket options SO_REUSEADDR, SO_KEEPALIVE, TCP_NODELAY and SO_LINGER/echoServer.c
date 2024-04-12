/* echoServer.c - TCP echo server program */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01g,06nov97,mm   added copyright.
01f,29Sep97,mm   changed printf  
01e,29Sep97,mm   cast arg1 of `bzero', cast arg 4 of `setsockopt', cast arg 2 of                 `bind', cast arg 2 of `accept'
01d,26Sep97,mm   added forward declaration STATUS echoTcpForever
01c,17Sep97,mm   deleted variable int numToRead from the function echoTcpForever
01b,17Sep97,mm   added include <stdio.h> <errnoLib.h> <usrLib.h> 
01a,07Feb94,ms   cleaned up for VxDemo.
*/

#include "sockLib.h"
#include <socket.h>
#include <errnoLib.h>
#include <usrLib.h>
#include <string.h>
#include <stdio.h>
#include "in.h"
#include "ioLib.h"
#include "netinet/tcp.h"

LOCAL int echoServerPort = 7001; /* default port num - should be greater */

/* Change the default keepidle time (timeout for probing the idle connection)
 * from several hours to few minutes.
 */
IMPORT int tcp_keepidle = 1; 


/*****************************************************************************
 * echoTcpServerRun - Runs the TCP server task for echo service
 *
 * DESCRIPTION
 *
 *     The TCP echo service specifies that a server must accept incoming
 *     requests, read data from the connection, an write the data back 
 *     over the connection until the client terminates the transfer.
 *
 *     Echo server merely returns all the data it receives from a client.
 *     Echo services are important tools that network managers use to test
 *     reachability (to make sure the connection works), debug protocol 
 *     software, and identify routing problems.
 *
 *     This also demonstrates the usage of SO_REUSEADDR, SO_KEEPALIVE,
 *     TCP_NODELAY and SO_LINGER set socket options.
 * 
 * EXAMPLE:
 *
 *     Run this echoTcpServerRun task on one VxWorks system as follows
 *     before starting the echoTcpClientRun task from another VxWorks system.
 *     The other half of the demonstration is in echoClient.c.
 *
 *     sp (echoTcpServerRun, port)
 *
 *     where port is the port number used by the echoTcpServer (should be
 *     greater than 5000).
 *
 *     example:
 *     -> sp (echoTcpServerRun, 7001)
 *
 */

/* forward declaration */

STATUS echoTcpForever();

void echoTcpServerRun 
    (
    int port			/* port number; 0 to use the last used one */
    )
    {

    if (port != 0)
	echoServerPort = port;

    FOREVER
	if (echoTcpForever () == ERROR)
            {
            printf ("echoTcpServerRun: echoTcpForever failed\n");
            return;
            }
    }


/*****************************************************************************
 * echoTcpServer - the TCP server task for echo service
 *
 */

STATUS echoTcpForever ()

    {
    struct sockaddr_in	serverAddr;            /* server's address */
    struct sockaddr_in	newConnAddr;    /* client's address */
    int 		sock;           /* socket fd */
    int 		newConnection;  /* socket fd */
    int 		len;            /* length of newConnAddr */ 
    int 		numRead;        /* number of bytes read */
    int 		optionVal;      /* value of setsocket option */
    struct linger 	linger;         /* amount of time to SO_LINGER */
    char 		buffer [1024];  /* data buffer */

    /* Zero out the sock_addr structures.
     * This MUST be done before the socket calls.
     */
    bzero ((char *) &serverAddr, sizeof (serverAddr));
    bzero (buffer, sizeof (buffer));

    /* open the TCP socket */
    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
	{
	perror ("socket");
	return (ERROR);
	}

     optionVal = 1; /* Turn ON the diff. setsockopt options */

    /* Specify the SO_REUSEADDR option  to bind a stream socket to a local  
     * port  that may be still bound to another stream socket that may be 
     * hanging around with a "zombie" protocol control block whose context
     * is  not yet freed from previous sessions.
     */
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &optionVal,
		    sizeof(optionVal)) == ERROR)
	{
	perror ("echoTcpForever: setsockopt SO_REUSEADDR failed");
	close (sock);
	return (ERROR);
	}

    /* Specify the SO_KEEPALIVE option, and the transport protocol (TCP)  
     * initiates  a timer to detect a dead connection which prevents an
     * an application from  hanging on an invalid connection.
     */
    if (setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &optionVal,
		    sizeof(optionVal)) == ERROR)
	{
	perror ("echoTcpForever: setsockopt SO_KEEPALIVE failed");
	close (sock);
	return (ERROR);
	}

    /* Specify the TCP_NODELAY option for protocols such as X Window System 
     * Protocol that require immediate delivery of many small messages. 
     *
     * By default VxWorks uses congestion avoidance algorithm 
     * for virtual  terminal  protocols and  bulk  data  transfer  
     * protocols. When the TCP_NODELAY option is turned on and there are 
     * segments to be sent out, TCP  bypasses  the  congestion
     * avoidance  algorithm  and sends the segments out if there 
     * is enough space in the send window.
     */
    if (setsockopt (sock, IPPROTO_TCP, TCP_NODELAY, (char *) &optionVal,
		    sizeof(optionVal)) == ERROR)
	{
	perror ("echoTcpForever: setsockopt TCP_NODELAY failed");
	close (sock);
	return (ERROR);
	}

    
    linger.l_onoff = 1;  /* Turn ON the SO_LINGER option */
    linger.l_linger = 0;/* Use default amount of time to shutdown i.e
                          the default value of TCP_LINGERTIME in tcp_timer.h*/ 

    /* Specify the SO_LINGER option to perform a  "graceful" close.
     * A graceful close occurs when a connection is shutdown, TCP  will  
     * try  to  make sure that all the unacknowledged data in transmission 
     * channel are acknowledged and the peer is shutdown properly by going 
     * through an elaborate  set  of  state transitions. 
     */
    if (setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof (linger))
	== ERROR)
	{
	perror ("echoTcpForever: setsockopt SO_LINGER failed");
	close (sock);
	return (ERROR);
	}

    /* Set up our internet address, and bind it so the client can connect. */
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = htons (echoServerPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    printf ("\nBinding SERVER :%x\n", serverAddr.sin_port);
    if (bind (sock, (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0)
	{
	perror ("bind");
	close (sock);
	return (ERROR);
	}

    /* Listen for the client to connect to us. */
    printf ("Listening to client\n");
    if (listen (sock, 5) < 0)
	{
	perror ("listen");
	close (sock);
	return (ERROR);
	}

    /* The client has connected.  Accept, and receive chars */
    len = sizeof (newConnAddr);
    newConnection = accept (sock, (struct sockaddr  *) &newConnAddr, &len);
    if (newConnection == ERROR)
        {
        perror ("accept failed");
        close (sock);
        return (ERROR);
        }

    /*  read data from the connection, and write the data back 
     *  over the connection until the client terminates the transfer.
     */
		
    while ((numRead = recv (newConnection, buffer, sizeof (buffer), 0)) > 0)
	{
	    printf ("buffer is - %s and numRead = %d\n", buffer, numRead);
	    send (newConnection, buffer, numRead, 0);
	 /*   buffer[0] = '\0';
	    numRead = 0;*/
        } 
    /* When the peer end of the TCP socket is closed, recv system 
     * call will return 0 bytes (means  EOF in TCP). When the connection is  
     * broken, recv will return ERROR with errno set to ETIMEDOUT.
     */
    numRead = recv (newConnection, buffer, sizeof (buffer), 0);

    if (numRead == 0)
        {
        printf ("No more bytes to read \n");
        printf ("Closing the sockets\n");
        }
    else
        {
        printErrno (errnoGet ());
        close (newConnection);
        close (sock);
        return (ERROR);
        }

    /* close the sockets */
    close (newConnection);
    close (sock);
    return (OK);
    }
