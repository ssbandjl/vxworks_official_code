FILE LIST -
             winBlaster.c

DESCRIPTION -
	     This is a client program which connects to the server via TCP
	     socket.  It allows to configure the maximum  size  of  the 
	     socket-level send buffer. It repeatedly sends a given size message
	     to the given port at destination target.
             
             NOTE - Currently there is no winBlastee, but you can use 
             vxBlastee on the vxWorks target. If you need a blastee on 
             the windows host port the unix or the vxWorks blastee to 
             Windows host. 

RUNNING DEMO - 
	     To run this blaster program from your Windows NT or Windows 95 
             host do the following at a DOS prompt:
	     C:\> blaster <target name>  7000  1000  16000

EXAMPLE COMPILE PROCEDURE -
	     blaster.exe was compiled using Microsoft Visual C++ 4.1 in the
             Microsoft Developer Studio.  

	     STEP 1:  Cretae a Project Workspace.
		Select File -> New... 
		Select Project Workspace and click 'OK'
		Select Type 'Console Application'.
		Enter Name: blaster
		Click 'Create'

	     STEP 2:  Edit Build Settings.
		Select Build -> Settings...
		Select the tab labeled 'Link'
		Under Object/Library modules add: wsock32.lib
		Click 'OK'

	     STEP 3:  Add Source file to Project Workspace.
		Select Insert -> Files Into Project...
		Enter File name: winBlaster.c		

             STEP 4:  Build the executable.
		Slecect Build -> Build blaster.exe

TESTED ON - winBlaster on NT 4.0 and Windows 95
            vxBlastee on mv162 
 

