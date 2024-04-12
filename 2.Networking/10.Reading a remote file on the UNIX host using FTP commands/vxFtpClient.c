/* vxFtpClient.c - Demo for using FTP communication between FTP client
 *                 running on a VxWorks target and  remote FTP server
 *                 running on a UNIX host 
 */
 
/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm     added copyright.
01b,15Sep97,mm     added <errno.h> <stdio.h> <unistd.h>
01a,18feb94,ms     written 
*/
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "vxWorks.h"
#include "ftpLib.h"
 
#define UNIX_HOST  "192.1.200.32"    /* inet address of the remote UNIX host */
#define USER       "john"              /* your login user name */
#define PASSWD     "iluvwrs"           /* your login passwd    */
#define W_DIR     "/sevana1/john/VxDemo/net/ftp/" /* cd to working directory */

#define RD_CMND    "RETR %s"           /* read a remote file FTP command */
#define QUIT_CMND  "QUIT"              /* quit FTP command */
#define FILE       "ftpData.txt"       /* file to request - located in W_DIR */

/*****************************************************************************
 * ftpReadDemo -  reads a remote file on a UNIX host using FTP commands
 * 
 *  CONFIGURATION:
 *
 *  You need to set/change the value of the UNIX_HOST, USER, PASSWD and W_DIR 
 *  constants given in this file.
 *  
 *  EXAMPLE:
 *
 *     To run ftpReadDemo, from the VxWorks shell do as follows:
 *     -> sp ftpReadDemo
 *
 *  RETURNS: OK or ERROR
 */

STATUS ftpReadDemo ()
    {
    int ctrlSock;
    int dataSock;
    char buffer [512];
    int numBytes;
 
    buffer [0] = '\0';
    /* initiate a transfer via a remote FTP server to read a remote file */
    if (ftpXfer (UNIX_HOST, USER, PASSWD, "", RD_CMND, W_DIR, 
                                    FILE, &ctrlSock, &dataSock) == ERROR)
        { 
        perror ("Error in initiating a transfer via a remote FTP server");  
        return (ERROR);
        }
    /* read the remote file - for this example it is assumed that the amount
     * of buffer that needs to be read is less than 512 bytes 
     */
    while ((numBytes = read (dataSock, buffer, sizeof (buffer))) > 0)
        {
        /* Do some processing here */
        printf ("\nData read: ");
        write (STD_OUT, buffer, numBytes);
        printf ("\n");
        }
    if (numBytes < 0)
        {
        perror ("Error in reading");          /* read error */
        return (ERROR);
        }
    close (dataSock);
    /* Get an FTP command reply to see whether EOF is encountered */
    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
        {
        perror ("positive completion failed");
        return (ERROR);
        }
    /* Send QUIT FTP command */
    if (ftpCommand (ctrlSock, QUIT_CMND, 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
        {
        perror ("QUIT FTP command positive completion failed");
        return (ERROR);
        }
    close (ctrlSock);
    return (OK);
    }



