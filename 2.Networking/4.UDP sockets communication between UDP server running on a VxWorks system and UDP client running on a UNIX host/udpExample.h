/* udpExample.h - header file for UDP example */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm   added copyright.
01a,10Feb94,ms   cleaned up for VxDemo.
*/

/* Defines common information for both server and client.*/

#define SERVER_PORT_NUM (5001)
#define MSG_SIZE (1024)

/* Structure used for request/message from client */

struct request
    {
    int reply; /* TRUE if client wants a reply */
    char message[MSG_SIZE];	/* Message buffer */
    };

