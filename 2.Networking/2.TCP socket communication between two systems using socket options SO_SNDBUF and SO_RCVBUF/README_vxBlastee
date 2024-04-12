FILE LIST -
      vxBlastee.c

DESCRIPTION -
	     This is a server program which communicates with client through a
	     TCP socket. It allows to configure the maximum size of socket-level
	     receive buffer. It repeatedly reads a given size message from the client
	     and reports the number of bytes read every minute. It stops receiving
	     the message when the global variable blasteeStop is set to 1.

RUNNING DEMO -
	     Place the  vxBlastee.c file in your
	     <Tornado> dir. Depending on the target
	     you have, define the CPU in your compile line and use
	     the Makefile in the BSP dir. to compile.

	     To run this code invoke the function blastee().

TESTED ON -  Host/Target : Solaris 2.5.1 / mv1604
	     VxWorks     : 5.3.1

	     EXAMPLE COMPILE LINE - 
			   make CPU=PPC604 vxBlastee.o

%  make CPU=PPC604 vxBlastee.o
ccppc -B/petaluma1/mayur/tor101-ppc/host/sun4-solaris2/lib/gcc-lib/ 
-mstrict-align -ansi -nostdinc -O2 -fvolatile -fno-builtin -fno-for-scope 
-Wall -I/h   -I. -I/petaluma1/mayur/tor101-ppc/target/config/all 
-I/petaluma1/mayur/tor101-ppc/target/h 
-I/petaluma1/mayur/tor101-ppc/target/src/config 
-I/petaluma1/mayur/tor101-ppc/target/src/drv -DCPU=PPC604  -DMV1600 
-DTARGET_DIR="\"mv1604\""   -c vxBlastee.c

OUTPUTS/LOGFILE -

On VxWorks target:
==================

-> ld <vxBlastee.o
value = 933240 = 0xe3d78
-> sp (blastee, 7000, 1000, 16000)
task spawned: id = 5c7c50, name = u21
value = 6061136 = 0x5c7c50

