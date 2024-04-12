/* nfsWrite.c - Demo for writing data to NFS file on UNIX host */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm   added copyright.
01b,16Sep97,mm   added #include <string.h> and #include <stdlib.h>
01a,02mar93,ms   written
*/
#include <string.h>
#include <stdlib.h>
#include "vxWorks.h"
#include "stdioLib.h"
#include "ioLib.h"

#define DATA_FILE_NAME "/petaluma1/mayur/Net-project/R_Dir/net/nfs/nfsWrite.dat" /* data file name */
#define DATA_FILE_MODE 0755       /* mode of data file (UNIX chmod style) */
#define TWO            2          /* Maximum two characters to write the line
                                   * numbers in the NFS data file 
                                   */ 

/****************************************************************************
 * nfsWrite - Writes data to NFS file on UNIX host
 *
 * RETURNS: OK or ERROR
 *
 * CONFIGURATION
 * You need to set/change the value of the DATA_FILE_NAME constant to the full
 * path name of the NFS data file that you want to create.
 *
 * EXAMPLE
 * To run this program from the VxWorks shell do as follows:
 *
 *  -> sp (nfsWrite)
 */


STATUS nfsWrite ()
    {
    int fd;
    int count;
    char *buffer;
    char msg[] = ": Hello World !!! \n"; 

    if ((buffer = (char *) malloc (strlen (msg) + TWO)) == NULL)
        {
        perror ("malloc failed");
        return(ERROR);
        }

    fd = open (DATA_FILE_NAME, O_CREAT | WRITE, DATA_FILE_MODE);
    if (fd < 0)
	{
        perror ("file open failed ");
        free (buffer);
	return (ERROR);
	}

    printf ("Writing to NFS file \n");
    for (count = 0; count < 11; count++)
	{
	sprintf (buffer, "%d%s", count, msg);
	if (write (fd, buffer, strlen (buffer)) == ERROR) 
            {
            perror ("write failed");
            return (ERROR);
            }
        else
	    {
            /* VxWorks caches the write request in NFS client code.
             * The default cache size is 8192. To write the data
             * immediately to the file without buffering, FIOSYNC option 
             * is used here to flush the NFS write cache.
             */

            if (ioctl (fd, FIOSYNC, 0) == ERROR)
                {
                perror ("FIOSYNC ioctl option failed");
                return (ERROR);
                }
	    printf (".\n");
	    }
        }

    free (buffer);
    close (fd);
    printf ("Completed nfsWrite\n");
    return (OK);
    }
