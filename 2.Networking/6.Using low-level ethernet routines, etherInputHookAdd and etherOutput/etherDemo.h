/* etherDemo.h - header for etherInputDemo and etherOutputDemo */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01a,17feb94,ms  written
*/

#define  ET_TYPE    (u_short)1111  /* user defined protocol type */
#define  INTERFACE_NAME "sm0" /* name of your network interface name*/
LOCAL STATUS etBcast(); /* broadcast on ethernet network interface */
/* LOCAL BOOL etHandle ();  ether input hook routine for handling input */
LOCAL char etMessage [] = "Hello World!!!"; /* data to be passed*/

/* Broadcast ethernet address is used for simple demonstration. You can
 * replace the value of the etbcastaddr variable with the destination 
 * ethernet address.
 */
unsigned char etbcastaddr [6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


