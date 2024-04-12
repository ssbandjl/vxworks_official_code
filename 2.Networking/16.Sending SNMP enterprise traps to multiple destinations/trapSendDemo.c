/* trapSendDemo.c - Send a SNMP enterprise trap to multiple destinations */

/* Copyright 1984-1997 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------

02b,11aug97,rv documentation 
02a,11aug97,rv use an array of pointers to destination ipaddresses 
01b,08aug97,rv fixed, included the header files from snmpIoLib.c  
01a,08aug97,rv written, sends a trap to sysBootHost 
*/

/*
DESCRIPTION
This example is based on the SNMPv1/v2c Component Release Supplement 1.0 
for Tornado 1.0.1
                  Section 4.7 Traps
                  Example 4-1 Trap Example

The code below provides a simple example of a trap. Invoking the function
trapExSend() causes an enterprise specific trap to be sent from the SNMP 
agent to the boot-host and two other trap destination hosts as defined 
below by the user.

CAVEAT
All the destination hosts to which the SNMP trap will be sent must be 
in the host-table. Use hostShow(), if the destination host is not listed 
then do a hostAdd() for the Hostname and Ipaddress on the VxWorks target. 
Add appropriate routes for the destination host on the vxWorks target 
as necessary.
*/

/* includes */

#include <vxWorks.h>
#include <sys/socket.h>
#include <snmpdLib.h>
#include <snmpIoLib.h>
#include <hostLib.h>
#include <inetLib.h>

/* defines */

#define MY_OIDLEN    0                /* length of snmpTrapMyOid */ 
#define TRAP_PORT    SNMP_TRAP_PORT   /* port number 162, snmpdefs.h */
#define SNDR_IPADDR  "147.11.41.159"  /* SNMP agent's ipaddress */
#define NUM_DEST     3                /* number of hosts receving the trap */ 
#define HOST0        "ganges"         /* trap destination host */ 
#define HOST1        "red"            /* trap destination host */

/* globals */

/* locals */

/* forward declarations */

int trapDestGet(int);
void trapExSend(void);
int trapBindVals();


/*******************************************************************************
* 
* trapBindVals - bind values in the trap
*
* This routine binds an integer and an octet-string variable in the 
* variable-binding list of the SNMP trap packet. The null cookie can be used
* for user specific additional information.   
*
* RETURNS: OK, or ERROR if the bind to the SNMP packet fails
*/
 
int trapBindVals
   (
   SNMP_PKT_T *    pkt,    /* internal representation of SNMP packet */ 
   void *          vals    /* null cookie */
   )
   {
   static OIDC_T   compl_1 [] = { 1, 3, 6, 1, 4, 1, 731, 1, 0 };
   static OIDC_T   compl_2 [] = { 1, 3, 6, 1, 4, 1, 731, 2, 0 };
   static OCTET_T  name [] = "Wind River SNMP";
 
   /* Do the bindings with index 0 and 1 respectively */
 
   if ((SNMP_Bind_Integer (pkt, 0, sizeof compl_1 / sizeof compl_1 [0], compl_1, 
       1997)) == -1)
       return (ERROR);

   if ((SNMP_Bind_String  (pkt, 1, sizeof compl_2 / sizeof compl_2 [0], compl_2, 
       VT_STRING, strlen (name), name, 1) == -1))
       return (ERROR);
 
   return (OK);
   }

/*******************************************************************************
* 
* trapExSend - send a SNMP enterprise specific trap to each destination host 
*
* This routine obtains the agent's ipaddress, sets the family, port and
* ipaddress of the destination hosts. It then sends a SNMPv1 trap to all 
* the destination hosts.
*   
* RETURNS: N/A
*/

void trapExSend(void)
{
    void *                 pDestAddr[NUM_DEST]; /* array of pointers to hosts */ 
    struct sockaddr_in     destAddr[NUM_DEST];  /* inet. domain addr. struct */ 
    u_long                 ipAddr;              /* trap sender's ipaddress */ 
    IMPORT int             snmpSocket;          /* socket descriptor for snmp */
    int                    count;               /* count num. of destinations */
 

    /* agent's ipaddress converted from dot to long integer in network order */  

     ipAddr = inet_addr(SNDR_IPADDR);  
 
     for (count = 0; count < NUM_DEST; count++)
     {
        destAddr[count].sin_family = AF_INET;
        destAddr[count].sin_port = htons (TRAP_PORT);
        destAddr[count].sin_addr.s_addr = trapDestGet(count);
        pDestAddr[count] = & destAddr[count];
     } 
 
      /* We send a trap with 2 varbinds as specified in the 3'rd last
      * param using the rtn trapBindVals to do the binding. A null
      * cookie is passed to the binding rtn in this particular case.
      * You may make use of it if needed.
      */
 
     snmpdTrapSend (&snmpSocket, NUM_DEST,  pDestAddr, NULL, SNMP_VERSION_1, 
         "trap community", NULL, MY_OIDLEN, &ipAddr, 
         ENTERPRISE_SPECIFIC, 0, 2, trapBindVals, 0);
} 

/*******************************************************************************
*
* trapDestGet - get the ipaddress of the remote hosts 
*
* This routine uses the name of the host listed in this funciton and finds  
* it's ipaddress from the host table added by hostAdd() in the vxworks target. 
* It returns the ipaddress as an integer.  
*
* RETURNS: ipaddress in integer form or ERROR if host is invalid.  
*/

int trapDestGet(int hostCount)
{
    IMPORT char     sysBootHost [];     /* host from which target is booted */


    if (hostCount == 0)
        return (hostGetByName (HOST0));
    else if (hostCount == 1)
        return (hostGetByName (HOST1));
    else
        return (hostGetByName (sysBootHost));
}

