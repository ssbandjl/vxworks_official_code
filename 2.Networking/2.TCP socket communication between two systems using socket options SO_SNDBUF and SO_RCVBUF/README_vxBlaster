FILE LIST -
      vxBlaster.c

DESCRIPTION -
	     This is a client task which connects to the server via TCP socket.
             It allows to configure the maximum  size  of  the  socket-level
	     send buffer. It repeatedly sends a given size message to the given port
	     at destination target. It stops sending the message when the global
	     variable blasterStop is set to 1.

RUNNING DEMO -
	     Place the vxBlaster.c file in your
	     <Tornado> dir. Depending on the target
	     you have, define the CPU in your compile line and use
	     the Makefile in the BSP dir. to compile.

	     To run this code invoke the function blaster().

TESTED ON -  Host/Target : Solaris 2.5.1 / mv1604
	     VxWorks     : 5.3.1

	     EXAMPLE COMPILE LINE -
			   make CPU=PPC604 vxBlaster.o

%  make CPU=PPC604 vxBlaster.o
ccppc -B/petaluma1/mayur/tor101-ppc/host/sun4-solaris2/lib/gcc-lib/ 
-mstrict-align -ansi -nostdinc -O2 -fvolatile -fno-builtin -fno-for-scope 
-Wall -I/h   -I. -I/petaluma1/mayur/tor101-ppc/target/config/all 
-I/petaluma1/mayur/tor101-ppc/target/h 
-I/petaluma1/mayur/tor101-ppc/target/src/config 
-I/petaluma1/mayur/tor101-ppc/target/src/drv -DCPU=PPC604  -DMV1600 
-DTARGET_DIR="\"mv1604\""   -c vxBlaster.c

OUTPUTS/LOGFILE -

On VxWorks target:
==================

 ld <vxBlaster.o
 value = 942704 = 0xe6270
 -> sp (blaster, "147.11.41.154", 7000, 1000, 16000)
 task spawned: id = 5c2c28, name = u22
 value = 6040616 = 0x5c2c28

The Output you see on the console window:
=========================================

Binding SERVER
Listening to client

host 147.11.41.4 port 7000
interrupt: No bytes read in the last 60 seconds.
interrupt: 2535241 bytes/sec
interrupt: 5997000 bytes/sec
interrupt: 6020300 bytes/sec
interrupt: 6009366 bytes/sec
interrupt: 5989783 bytes/sec
interrupt: 6036600 bytes/sec
interrupt: 6008366 bytes/sec
interrupt: 6019116 bytes/sec
blaster exit.
interrupt: 5844825 bytes/sec
interrupt: No bytes read in the last 60 seconds.
interrupt: No bytes read in the last 60 seconds.
blastee end.

