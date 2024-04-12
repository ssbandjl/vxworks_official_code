/* setTimeOfDay.c - Demo for setting the clock to the time of the day value */

/* Copyright 1991 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,10aug97,whjr  modified and documented
01a,01Mar94,ms   cleaned up for VxDemo
,ldt  Cleaned up for 5.1.1 VxWorks
,ldt  Added UNIX compatibility
*/

/*
DESCRIPTION
TIMEZONE: 
This file sets the time zone environment  variable  TZ,  and
the  locale-related  environment variables LANG, LC_COLLATE,
LC_CTYPE, LC_MESSAGES, LC_MONETARY, LC_NUMERIC, and LC_TIME.
stdio:
A file with associated buffering is  called  a  stream  (see
intro(3))  and is declared to be a pointer to a defined type
FILE.  fopen() creates certain descriptive data for a stream
and returns a pointer to designate the stream in all further
transactions.  Normally, there are three open  streams  with
constant pointers declared in the <stdio.h> header and asso-
ciated with the standard open files:
date:
The date utility writes the date and time to standard output
or  attempts  to  set the system date and time.  By default,
the current date and time will be written.
NOTES: 
If you attempt to set the current date to one of  the  dates
that the standard and alternate time zones change (for exam-
ple, the date that daylight time is starting or ending), and
you  attempt  to  set  the  time  to  a time in the interval
between the end of standard time and the  beginning  of  the
alternate  time  (or  the  end of the alternate time and the
beginning of standard time), the results are unpredictable.

*/


#ifdef UNIX

#include "unix_compat.h"
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>	

#else

#include "vxWorks.h"
#include "ioLib.h" /* @@@ ldt */
#include "sockLib.h" /* @@@ ldt */
#include "hostLib.h" /* @@@ ldt */
#include "envLib.h" /* @@@ ldt */
#include "timers.h"
#include <socket.h>	

#endif /* UNIX */

#include "netinet/in.h"
#include "time.h"
#include "stdio.h" /* @@@ ldt */
#include "string.h" /* @@@ ldt */


#define IPPORT_TIMESERVER 37               /* port number for host timeserver*/

/*
#define TIME_ENV "TIMEZONE = PST:PDT:480:040202:101502"  
As part of the work-around, set PDT start and stop times to be the gmt times
  that PDT starts and stops locally.
*/
#define TIME_ENV "TIMEZONE = PST:PDT:480:040202:101502"  

#ifdef __cplusplus
extern "C" {
#endif

IMPORT char sysBootHost [];     /* VxWorks saves name of host booted from */

#if defined(__STDC__) || defined(__cplusplus)

STATUS setTimeOfDay (time_t); /* set the clock to the time of the
                                      * day value */   
STATUS getRemTime (time_t *, char *);   /* get the time of the day value
                                       * from remote UNIX boot host */
STATUS date();

#else   /* __STDC__ */

STATUS setTimeOfDay ();
STATUS getRemTime ();
STATUS date();

#endif  /* __STDC__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*****************************************************************************
 * rdate - Demonstrate setting the clock to the time of the day 
 *                    value got from the remote UNIX boot host, and print
 *                    the current time of the day value.
 *
 *  CONFIGURATION:    You need to set the value of the TIME_ENV constant 
 *                    to set the TIMEZONE environment variable.
 *  EXAMPLE:
 *
 *     To run this from the VxWorks shell do as follows:
 *     -> sp (rdate)
 *
 *  RETURNS: OK or ERROR
 */

STATUS rdate (remoteHost)

    char *remoteHost;    /* Name of the remote unix host. If NULL uses the
                         * name of UNIX boot host */
{
    time_t curTime;
    
    /* put the environment variable for TIMEZONE */
    putenv (TIME_ENV);

    /* Get time of the day value from the remote UNIX boot host */
    if (getRemTime (&curTime, remoteHost) != OK)
    {
        return ERROR;
    }

    /* Set and print the time of the day value */
    if (setTimeOfDay (curTime) == OK)
    {
        return (date ());
    }
    
   return ERROR;
}

/*****************************************************************************/
void diagnoseTime (timeNow)
    time_t timeNow;
{
/* set TZ */

    struct tm *tms;

    printf ("timeNow = 0x%x\n", (unsigned int) timeNow);

    tms = localtime(&timeNow);

    printf ("tms      = 0x%x\n", (unsigned int) tms);
    printf ("tm_sec   = %3d\n", tms->tm_sec);
    printf ("tm_min   = %3d\n", tms->tm_min);
    printf ("tm_hour  = %3d\n", tms->tm_hour);
    printf ("tm_mday  = %3d\n", tms->tm_mday);
    printf ("tm_mon   = %3d\n", tms->tm_mon);
    printf ("tm_year  = %3d\n", tms->tm_year);
    printf ("tm_wday  = %3d\n", tms->tm_wday);
    printf ("tm_yday  = %3d\n", tms->tm_yday);
    printf ("tm_isdst = %3d\n", tms->tm_isdst);
}

/*****************************************************************************
 * date - print the time of the day value 
 *
 */

STATUS date()
{
    time_t    timeNow;
    struct tm timeBuffer;

    if ((timeNow = time ((time_t *) NULL)) == ERROR)
    { 
        perror ("date: Error in determining the current calender time");
        return ERROR;
    }

#ifdef UNIX

    printf (ctime (&timeNow));

#else /* UNIX */

    /* Work-Around for ctime/localtime bug */

    if (gmtime_r (&timeNow, &timeBuffer) != OK)
    {
        perror ("date: Error in gmtime");
        return ERROR;
    }
    timeBuffer.tm_isdst = -1; /* work around bug in internals */

/*
    Keep in mind that local to daylight time transition will happen
    relative to GMT with this work-around
*/
    if (localtime_r (&timeNow, &timeBuffer) != OK)
    {
        perror ("date: Error in converting to local time");
        return ERROR;
    }
    printf (asctime (&timeBuffer));

#endif /* UNIX */

    return OK;
}

/*****************************************************************************
 * setTimeOfDay - set the clock to the time of the day value 
 *
 *
 *  RETURNS: OK or ERROR
 */

STATUS setTimeOfDay
    (
    newTime
    )
    time_t newTime;
    {

#ifdef UNIX
    printf ("Not setting time of day on Unix machine\n");
    return (OK);
#else
    struct timespec curTime;

    /* There is a 2208988800 seconds  difference between 0 in UNIX time 
     * (Jan 1 1900 since 00 GMT) and what the inetd time service gives
     * (elapsed time in seconds since 00 GMT Jan 1 1970).  Hence the
     * use of this magic number.
     */

    curTime.tv_sec = (ntohl (newTime) - 2208988800);
    curTime.tv_nsec = 0;
    if (clock_settime (CLOCK_REALTIME, &curTime) == ERROR)
        {
        perror ("setTime: Error in setting the clock value");
        return (ERROR);
        }
    return (OK);
#endif /* UNIX */
}

/*****************************************************************************
 * getRemTime - get the time of the day from the remote UNIX host 
 *
 *
 *  RETURNS: time of the day from the remote UNIX host as number of seconds 
 *           elapsed since 00 GMT Jan 1 1970.
 *
 */


STATUS getRemTime 
    (
    curTime,
    remoteHost
    )
    char *remoteHost;    /* Name of the remote unix host. If NULL uses the
                         * name of UNIX boot host */
    time_t *curTime;
{

#ifdef UNIX
    printf ("not getting remote time for UNIX\n");
    *curTime = (time_t) time(NULL);
    return (OK);
#else
    struct sockaddr_in sockaddr;
    int sockFd;
    time_t remTime;
    char *buffer;
    int nBytes;
    int nRead;

    if (remoteHost == NULL)
    {
        remoteHost = sysBootHost;
    }

    printf ("Setting date from: %s\n", remoteHost); /* @@@ ldt */

    bzero ((char *) (&sockaddr), sizeof (sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons (IPPORT_TIMESERVER);
    if ((sockaddr.sin_addr.s_addr = hostGetByName (remoteHost)) == ERROR)
        {
        perror ("getRemTime: unknown host");
        *curTime = (time_t) 0;
        return (ERROR);
        }

    if ((sockFd = socket (AF_INET, SOCK_STREAM, 0)) == ERROR) 
        {
        perror("getRemTime: socket failed.");
        *curTime = (time_t) 0;
        return (ERROR);
        }

    if (connect (sockFd, (struct sockaddr *) &sockaddr, 
                                                sizeof(sockaddr)) == ERROR) 
        {
        close (sockFd);
        perror ("getRemTime: connect failed.");
        *curTime = (time_t) 0;
        return (ERROR);
        }

    buffer = (char *)& remTime;
    nBytes = sizeof (remTime);

    while (nBytes) 
        {
        if ((nRead = read(sockFd, buffer, nBytes)) == ERROR ) 
            {
            close (sockFd);
            perror ("getRemTime: read failed.");
            *curTime = (time_t) 0;
            return (ERROR);
            }
        nBytes -= nRead;
        buffer += nRead;
        }
    close (sockFd);
    *curTime = (time_t) remTime;
    return (OK);

#endif /* UNIX */
}

#ifdef UNIX
int main ()
{
    rdate (NULL);
    return 0;
}
#endif /* UNIX */
