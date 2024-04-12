/* rdate.c - Demo for displaying the results of the date command executed on 
             a remote UNIX  host */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01a,16Sep97,mm   added include <stdio.h> <unistd.h> and <remLib.h>
01a,09Feb92,ms   written
*/

#include "vxWorks.h"
#include <remLib.h>
#include <stdio.h>
#include <unistd.h> 

#define RSH_PORT  514             /* rshd remote port number */
#define BUFSIZE  128              /* buffer size of the result */


/*****************************************************************************
 * rdate - display the results of the date command executed on your remote 
 *         UNIX  host 
 *
 *  EXAMPLE:
 *
 *     To run this rdate task, from the VxWorks shell do as follows:
 *     -> sp (rdate, "hostName") 
 *
 *     where hostName is the name of the UNIX host (as specified in
 *     the VxWorks target boot parameters).
 *
 *  RETURNS: OK or ERROR
 */


STATUS rdate
    (
    char *unixHost /* name of the remote UNIX host - same as the host 
                      specified in VxWorks target boot parameters */
    )
    {
    int rshdSockFd;     /* socket fd for communicating with UNIX host's rshd */
    char userName [20]; /* local and remote user name */
    char buf [BUFSIZE]; /* buffer for getting the return value of 
                         * remote date command */
    int nRead;          /* number of bytes read */


    remCurIdGet(userName, NULL); /* get the user name */
    printf (" Name of the user is %s\n", userName);

    if (unixHost == NULL)
        {
        printf ("Invalid host name\n");
        return (ERROR);
        }
         
    /* execute date shell command on your UNIX host */
    if ((rshdSockFd = rcmd (unixHost, RSH_PORT, userName, userName,
                                                "date",0)) == ERROR)
        {
        perror ("rcmd failed");
        return (ERROR);
        }

    /* read the return value of the date command and send it to stdout */
    if ((nRead = read(rshdSockFd, buf, BUFSIZE)) == ERROR)
        {
        perror ("read failed");
        return (ERROR);
        } 
    else 
        { 
        printf ("Number of bytes read %d \n",nRead);
        buf [nRead - 1] = '\0';
        printf ("\nDate on %s: %s \n", unixHost, buf);
        }

    close (rshdSockFd);
    return 1;
    }
