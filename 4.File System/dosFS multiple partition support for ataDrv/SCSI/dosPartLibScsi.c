/* dosPartLibScsi.c - show & mount SCSI HDD MSDOS partitions */


/*
modification history
--------------------
01d,24feb99,jkf   Fixed bug in scsiCmdBuild failing for large disks.
01c,15sep97,jkf   removed LCHS support.  Only useful for MSDOS versions
                  before 3.3, not likely anyone needs LCHS support.  
                  Added support for big endian arches and arches without
                  dynamic bus sizing, which involved byte swapping macro
                  and reading sector data with macro offsets. Now supports
                  80x86, 68k, SPARC, MIPS, PPC, i960, etc.)  Revised docs.
01b,01sep97,jkf   fixed stdlib.h filename case for Unix.
                  fixed cosmetics in sector and show routines.  
01a,01sep97,jkf   adapted from dosPartLibAta.c.
*/


/*
DESCRIPTION:

This library is provided by Wind River Systems Customer
Support strictly as an educational example.

This code supports both mounting MSDOS file systems and
displaying partition tables written by MSDOS FDISK.exe or
by any other MSDOS FDISK.exe compatible partitioning software.  

The first partition table is contained within a hard drives
Master Boot Record (MBR) sector, which is defined as sector
one, cylinder zero, head zero or logical block address zero.

The mounting and displaying routines within this code will 
first parse the MBR partition tables entries (defined below)
and also recursively parse any "extended" partition tables,
which may reside within another Cylinder, Head, and Sector 
(CHS) combination further into the hard disk.   (Also known
as extended partition tables chained from the MBR's partition
table.  MSDOS file systems within extended partitions are known
to those familiar with the MSDOS FDISK.exe utility as "Logical 
drives within the extended partition".

Since MSDOS file systems have a fixed number of clusters, 64Kb,
and clusters are the minimum allocation unit at the file system
level, using partitions can help optimize cluster size for a given
hard disk,  improving performance and resource usage.  For example, 
a 1-gigabyte hard disk with a single MSDOS file system has the 
following minimum allocation unit at the dosFsLib open/read/write
level interface:

    1GB/64KB = Minimum allocation unit.

This may produce a minimum allocation unit that is larger than
what the developer desires (for cluster boundary buffer alignment,
or other buffering, ect).

One solution is to sub-divide the disk into smaller partitions
or "logical disks" on the single physical disk and format file 
systems over each of the partitions.  (Then using this code you
may mount them.)   If you used four equal sized partitions, then
this method produces:

    256MB/64KB = Minimum allocation unit.	
    256MB/64KB = Minimum allocation unit.
    256MB/64KB = Minimum allocation unit.
    256MB/64KB = Minimum allocation unit.

This produces a smaller minimum allocation unit and may be 
better suited for optimal application file system performance.
Each partition contains its own unique file system.  As far
as the file system level is concerned it is on a unique disk.

Here is a picture showing the layout of a single disk containing
multiple MSDOS file systems:

(Note, all ASCII pictures herein are best viewed with
 an 'equally sized characters' or 'fixed width' style font)

  +---------------------------------------------------------+
  |<---------------------The entire disk------------------->|
  |M                                                        |
  |B<---C:--->                                              |
  |R           /---- First extended partition--------------\|
  |           E<---D:---><-Rest of the ext part------------>|
  |P          x                                             |
  |A          t          E<---E:--->E<Rest of the ext part->|
  |R                     x          x                       |
  |T                     t          t<---------F:---------->|
  +---------------------------------------------------------+
  (Ext == extended partition sector)


A MS-DOS partition table resides within one sector on a hard
disk.  There is always one in the first sector of a hard disk
partitioned with FDISK.exe.  There first partition table may
contain references to "extended" partition tables residing on
other sectors if there are multiple partitions.  The first 
sector of the disk is the starting point.  Partition tables
are of the format:

Offset from     
the beginning 
of the sector          Description
-------------          -------------------------
   0x1be               Partition 1 table entry  (16 bytes)
   0x1ce               Partition 2 table entry  (16 bytes)
   0x1de               Partition 3 table entry  (16 bytes)
   0x1ee               Partition 4 table entry  (16 bytes)
   0x1fe               Signature  (0x55aa, 2 bytes)


Individual MSDOS partition table entries are of the format:

Offset   Size      Description
------   ----      ------------------------------
 0x0     8 bits    boot type
 0x1     8 bits    beginning sector head value
 0x2     8 bits    beginning sector (2 high bits of cylinder#)
 0x3     8 bits    beginning cylinder# (low order bits of cylinder#)
 0x4     8 bits    system indicator
 0x5     8 bits    ending sector head value
 0x6     8 bits    ending sector (2 high bits of cylinder#)
 0x7     8 bits    ending cylinder# (low order bits of cylinder#)
 0x8    32 bits    number of sectors preceding the partition
 0xc    32 bits    number of sectors in the partition
  
In the partition table entry, the Sector/Cylinder (16bits)
(offset 0x2 & 0x3) fields data are stored in the format:

 |7|6|5|4|3|2|1|0|  The first 8 bits of offset 0x2.
  | | `------------ 6 least significant bit's contain the sector 
  | |               offset within cylinder.  sector == 6 bits.
  | |
  `-`-------------- Two most significant (high order) bits of the 
                    cylinder value.

 |7|6|5|4|3|2|1|0|  2nd 8 bits  (cylinder 0x3)
  `---------------- Eight least significant (low order) bits of the
                    cylinder value.

This format restricts partition entries to 10 cylinder bits & 6 sector
bits and 8 head bits.  This is also the format used by the PC-AT BIOS
Int13h functions, which is used by FDISK.exe 

dosPartLibAta/Scsi.c uses the following method of parsing the CHS values
stored in the 0x2 & 0x3 partition table offsets into distinct values:

Cyl = ((((dospt_startSec) & 0x00c0) << 2) |
       (((dospt_startSec) & 0xff00) >> 8));

Hd  = pPartTbl->dospt_startHead;

Sec = (pPartTbl->dospt_startSec & 0x003f); 

(see DOS_PART_TBL struct typedef in dosFsLib.h)

This produces individual CHS value from the values that
are contained within the MSDOS partition table entry.  

Definitions of CHS terms:

CHS - Cylinder/Head/Sector. 

L-CHS - Logical - CHS (used by INT 13 AxH/FDISK.exe) 
        256 heads, 1024 cylinders, 63 sectors. max.
        LCHS allow ~8.4 GB addressable space.

P-CHS - Physical CHS (used by physical ATA device) 
        16 heads, 65535 cylinders, 63 sectors.
        PCHS allow ~137GB addressable space.
        (VxWorks ataRawio() uses PCHS)

LBA - Logical Block Address. ATA/SCSI absolute sector
      numbering scheme.  LBA sector 0 == addresses the
      first sector on a device by definition.  ATA devices
      may also support LBA at the device interface. ATA
      standard says that cylinder 0, head 0, sector 1 == LBA 0.
      LBA allows (~137GB) on an ATA device. 

The  MSDOS "logical CHS" (LCHS) format combined with the 
Physical IDE/ATA CHS interface (PCHS) gives MSDOS the 
following addressing limitation:

                  +------------+----------+----------------+   
                  | BIOS INT13 | IDE/ATA  | Combined Limit |
+-----------------+------------+----------+----------------+   
|Max Sectors/Track|  63        |    255   |     63         |
+-----------------+------------+----------+----------------+
|Max Heads        |  256       |     16   |     16         |
+-----------------+------------+----------+----------------+
|Max Cylinders    |  1024      |  65536   |    1024        |
+-----------------+------------+----------+----------------+
|Capacity         |  8.4GB     |  136.9GB |   528 MB       |
+-----------------+------------+----------+----------------+

1024 x 16 x 63 X 512 bytes/sector = 528.48 MB limit.

Cylinder bits : 10 = 1024
Head bits     :  4 = 16
Sector bits   :  6 (-1) = 63 

(by convention, CHS sector numbering begins at
 #1, not #0; LBA begins at zero, which is CHS#1) 

To overcome the limitation. PC BIOS/OS vendors perform what is often
referred to as geometric or "drive" CHS translation to address more 
than 528MB, and to "address" cylinders above 1024.

BIOS vendors offer CHS translation extensions that are based
around the format of the PC-AT BIOS call "INT 0x13 function 0x8"
aka "Get Current Drive Parameters"  There are also two parameter
tables EDPT and FDPT set by the BIOS interfaces.  EDPT is sometimes
not supported depending on the system BIOS.  The FDPT is always 
supported on PC-AT compatible BIOS.  Code is available for the x86
VxWorks BSP to call INT13h F8h romInit.s BIOS call, which returns
the FDPT.  This is not needed when using LBA on newer systems.
It is possible in the pc486/pc386 VxWorks BSP's to make an
BIOS (INT13h F8H) call in romInit.s before the switch to 
protected mode.  Sample romInit.s code demonstrating this 
will be provided upon request, contact johnx@wrs.com.
requesting "Int13h romInit.s x86 code".

There exists also some 3rd party software based disk overlay
programs (Such as OnTrack's Disk Manager,TM.) which uses a
semi-proprietary strategy to support drives over 528MB.  

In most cases, the translated LCHS geometry is applied such that
all sectors are addressed at the same physical location as when
the drive is used within an untranslated environment.  However,
not in all cases, and then there is a breakdown.  Alas, there 
is not any standard for defining the altered geometry translation
scheme, so compatibility issues will arise when moving drives from
one BIOS, Operating System, driver, and/or controller to another.

VxWorks SCSI and ATA drivers communicate directly with the
controller and do not use the BIOS.  The VxWorks ATA driver
may address the full 136.9GB offered by ATA, and the full range
offered by SCSI.  So this is not an issue for VxWorks.  It should
be noted however, since all the partition table data will be stored 
in LCHS format.

Here is an example of a typical PCHS to LCHS translation.

(Note, all ASCII pictures herein are best viewed with
 an 'equally sized characters'/'fixed width' style font)

+----------------+------------+----------------+-------------+--------+
|Actual Cylinders|Actual Heads|Altered Cylinder|Altered Heads|Max Size|
+----------------+------------+----------------+-------------+--------+
|  1<C<=1024     | 1<H<=16    |     C=C        |    H=H      |528 MB  |
+----------------+------------+----------------+-------------+--------+
| 1024<C<=2048   | 1<H<=16    |     C=C/2      |    H=H*2    |  1GB   |
+----------------+------------+----------------+-------------+--------+
| 2048<C<=4096   | 1<H<=16    |     C=C/4      |    H=H*4    | 2.1GB  |
+----------------+------------+----------------+-------------+--------+
| 4096<C<=8192   | 1<H<=16    |     C=C/8      |    H=H*8    | 4.2GB  |
+----------------+------------+----------------+-------------+--------+
| 8192<C<=16384  | 1<H<=16    |     C=C/16     |    H=H*16   | 8.4GB  |
+----------------+------------+----------------+-------------+--------+
| 16384<C<=32768 | 1<H<=8     |     C=C/32     |    H=H*32   | 8.4GB  |
+----------------+------------+----------------+-------------+--------+
| 32768<C<=65536 | 1<H<=4     |     C=C/64     |    H=H*64   | 8.4GB  |
+----------------+------------+----------------+-------------+--------+


Another type of translation always sets the sectors to 63, 
this produces different geometry than the above.

+----------------+----------+---------+--------------+
|Range           | Sectors  | Heads   | Cylinders    |
+----------------+----------+---------+--------------+
|  1<X<=528MB    |   63     |  16     | X/(63*16*512)|
+----------------+----------+---------+--------------+
| 528MB<X<=1GB   |   63     |  32     | X/(63*32*512)|
+----------------+----------+---------+--------------+
| 1GB<X<=2.1GB   |   63     |  64     | X/(63*64*512)|
+----------------+----------+---------+--------------+
| 2.1GB<X<=4.2GB |   63     |  128    |X/(63*128*512)|
+----------------+----------+---------+--------------+
| 4.2GB<X<=8.4GB |   63     |  256    |X/(63*255*512)|
+----------------+----------+---------+--------------+

There are several MSDOS utilities (from Phoenix, and Western 
Digital, and more, such as wdtblchk.exe, and chkbios.exe) which
can be used on PC's to check translations.  These utilities
use BIOS calls to read the EPDT and FPDT to determine the PCHS
and LCHS and LBA values for a given system.  


TRANSLATION ALGORITHMS:
----------------------

The algorithm used in this code for LBA to PCHS translation 
(in dosPartRdLba[Ata/Scsi]) is:
        
PCHS_cylinder = LBA / (heads_per_cylinder * sectors_per_track)
         temp = LBA % (heads_per_cylinder * sectors_per_track)
PCHS_head     = temp / sectors_per_track
PCHS_sector   = temp % sectors_per_track + 1


The reverse, PCHS to LBA, is not implement but shown here
should anyone need to implement it:

LBA = (((cylinder * heads_per_cylinder + heads )
         * sectors_per_track ) + sector - 1)


Both of these can also be used with either LCHS or PCHS.
If using LCHS, then you must use LCHS drive parameters.
If using PCHS, then you must use PCHS drive parameters.

The "N-Shift" Algorithm used for LCHS to PCHS 
translation is:

   PCHS_cylinder = ( LCHS_cylinder * N ) + ( Lhead / NPH );
   PCHS_head = ( LCHS_head % NPH );
   PCHS_sector = LCHS_sector;



DEBUGGING:
----------
This code may be compiled with -g and debugged within Tornado.
Also the routine dosPartShowAta will show all partition data
for all partition table entries.  


SUPPORT:
--------
This code was written in my "spare" time.  I would feel it to be
viable code, so please send me (John Fabiani, johnx@wrs.com) any
comment or bug report, email only please.  This does not imply 
support for this code from Wind River Systems.  If you try to 
use this code without a thorough review, you are taking a risk.
This is considered only an example.  I will endeavor to keep it
working well. 



KNOWN LIMITATIONS
-----------------

1.) This code does not write partition tables, ala MSDOS FDISK.exe.
    This is the next thing I will be doing.  I have structured
    this code accordingly in anticipation of coding VxFdisk().

2.) This code must be spawned with an adequate stack size.
    dosPartParse is recursive and could use a lot of stack:

    To correctly spawn its routines from the shell:

    -> sp dosPartMountScsi, pSysScsiCtrl
    or

    -> taskSpawn ("pMount",20,0,20000,dosPartMountScsi,pSysScsiCtrl)
    or

    -> sp dosPartShowScsi,pScsiPhysDev
    or

    -> taskSpawn ("pShow", 20, 0, 20000, dosPartShowScsi,pScsiPhysDev)



    Do not call it directly from the shell ala:

    -> dosPartMountScsi pSysScsiCtrl -- WRONG!, may overflow default stack.
    -> dosPartShowScsi pScsiPhysDev  -- WRONG!, may overflow default stack.

    Or else you will most likely overflow your stack if you have
    not increased the default sp stack size.  This can be set
    with the WindShell via:

    -> ?     
    tcl> set spStackSize 100000
    100000
    tcl> ?
    -> 


3.) This code has only been tested with VxWorks 5.3.1 FCS.


COMPILING THIS CODE:
-------------------

Standard vxWorks makes are expect.  Should work with any arch.
Here are some examples:

68K:
----
cc68k -BD:\10168k/host/x86-win32/lib/gcc-lib/ -m68040 -ansi 
-nostdinc -O -fvolatile -fno-builtin -Wall -I/h   -I. 
-ID:\10168k\target\config\all -ID:\10168k\target/h 
-ID:\10168k\target/src/config -ID:\10168k\target/src/drv -DCPU=MC68040      
-c dosPartLibAta.c

cc68k -BD:\10168k/host/x86-win32/lib/gcc-lib/ -m68040 -ansi 
-nostdinc -O -fvolatile -fno-builtin -Wall -I/h   -I. 
-ID:\10168k\target\config\all -ID:\10168k\target/h 
-ID:\10168k\target/src/config -ID:\10168k\target/src/drv -DCPU=MC68040
-c dosPartLibScsi.c

I960:
-----
cc960 -BC:\Sirocco/host/x86-win32/lib/gcc-lib/ -mca -mstrict-align -ansi 
-nostdinc -O -fvolatile -fno-builtin  -Wall -I/h   -I. 
-IC:\Sirocco\target\config\all -IC:\Sirocco\target/h 
-IC:\Sirocco\target/src/config -IC:\Sirocco\target/src/drv -DCPU=I960CA 
-DVX_IGNORE_GNU_LIBS     -c dosPartLibScsi.c

80x86:
------
cc386 -BC:\Sirocco/host/x86-win32/lib/gcc-lib/ -m486 -ansi -nostdinc 
-O -fvolatile -nostdlib -fno-builtin -fno-defer-pop -Wall -I/h   -I. 
-IC:\Sirocco\target\config\all -IC:\Sirocco\target/h 
-IC:\Sirocco\target/src/config -IC:\Sirocco\target/src/drv -DCPU=I80486      
-c dosPartLibScsi.c



SOME GENERIC USAGE EXAMPLES:
------------------------------------------------------------------------------
    ***********************************************************
1.) EXAMPLE OF USING "dosPartMountAta (ctrl, drive)" ON A DRIVE
    CONTAINING TEN PARTITIONS OF VARIOUS SIZES: MSDOS C: - L: 
    ***********************************************************

    -> ld < c:\dosPartLib\dosPartLibAta.o
    value = 14364680 = 0xdb3008

    -> sp dosPartMountAta,0,0            
    task spawned: id = 1fb4788, name = u1
    value = 33245064 = 0x1fb4788
    -> 

    Mounting partition 00 - VxWorks device Name "/ataCtrl0Drive0Part0"
    Absolute sector: 0x0000003f  Number of sectors: 0x00005e41
    Approximate Partition size: 11 MBs

    Mounting partition 01 - VxWorks device Name "/ataCtrl0Drive0Part1"
    Absolute sector: 0x00005ebf  Number of sectors: 0x00018981
    Approximate Partition size: 49 MBs

    Mounting partition 02 - VxWorks device Name "/ataCtrl0Drive0Part2"
    Absolute sector: 0x0001e87f  Number of sectors: 0x0003b0c1
    Approximate Partition size: 118 MBs

    Mounting partition 03 - VxWorks device Name "/ataCtrl0Drive0Part3"
    Absolute sector: 0x0005997f  Number of sectors: 0x00001f41
    Approximate Partition size: 3 MBs

    Mounting partition 04 - VxWorks device Name "/ataCtrl0Drive0Part4"
    Absolute sector: 0x0005b8ff  Number of sectors: 0x00022701
    Approximate Partition size: 68 MBs

    Mounting partition 05 - VxWorks device Name "/ataCtrl0Drive0Part5"
    Absolute sector: 0x0007e03f  Number of sectors: 0x00018981
    Approximate Partition size: 49 MBs

    Mounting partition 06 - VxWorks device Name "/ataCtrl0Drive0Part6"
    Absolute sector: 0x000969ff  Number of sectors: 0x00013ac1
    Approximate Partition size: 39 MBs

    Mounting partition 07 - VxWorks device Name "/ataCtrl0Drive0Part7"
    Absolute sector: 0x000aa4ff  Number of sectors: 0x00044e41
    Approximate Partition size: 137 MBs

    Mounting partition 08 - VxWorks device Name "/ataCtrl0Drive0Part8"
    Absolute sector: 0x000ef37f  Number of sectors: 0x000a6581
    Approximate Partition size: 332 MBs

    Mounting partition 09 - VxWorks device Name "/ataCtrl0Drive0Part9"
    Absolute sector: 0x0019593f  Number of sectors: 0x00071301
    Approximate Partition size: 226 MBs

    -> devs
    drv name
      0 /null
      1 /tyCo/0
      1 /tyCo/1
      2 /pcConsole/0
      2 /pcConsole/1
      7 eno:
      8 /vio
      4 /ataCtrl0Drive0Part0
      4 /ataCtrl0Drive0Part1
      4 /ataCtrl0Drive0Part2
      4 /ataCtrl0Drive0Part3
      4 /ataCtrl0Drive0Part4
      4 /ataCtrl0Drive0Part5
      4 /ataCtrl0Drive0Part6
      4 /ataCtrl0Drive0Part7
      4 /ataCtrl0Drive0Part8
      4 /ataCtrl0Drive0Part9
    value = 0 = 0x0
    -> 

    -> dosFsConfigShow "/ataCtrl0Drive0Part6"

    device name:               /ataCtrl0Drive0Part6
    total number of sectors:   80577
    bytes per sector:          512
    media byte:                0xf8
    # of sectors per cluster:  4
    # of reserved sectors:     1
    # of FAT tables:           2
    # of sectors per FAT:      79
    max # of root dir entries: 512
    # of hidden sectors:       63
    removable medium:          TRUE
    disk change w/out warning: not enabled
    auto-sync mode:            not enabled
    long file names:           not enabled
    exportable file system:    not enabled
    lowercase-only filenames:  not enabled
    volume mode:               O_RDWR (read/write)
    available space:           41148416 bytes
    max avail. contig space:   32360448 bytes
    value = 0 = 0x0
    -> 


------------------------------------------------------------------------------
    ***********************************************************
2.) EXAMPLE OF USING "dosPartShowAta (ctrl, drive) ON DISK WITH
    MULTIPLE PARTITIONS. (some output deleted to save space)
    ***********************************************************
    
    -> sp (dosPartShowAta,0,0)                 
    task spawned: id = 1fb47b0, name = u1
    value = 33245104 = 0x1fb47b0
    -> 

    +--------------------------------------+
    | Master Boot Record - Partition Table |
    +--------------------------------------+----------+
    | Current LCHS Cylinder 0000, Head 000, Sector 00 |
    +-----------------------------+-------------------+---------+
    | Partition Entry number 04   | Partition Entry offset 0x1be|
    +-----------------------------+-----------------------------+
    | Status field = 0x80         | Primary (bootable) Partition|
    +-----------------------------+-----------------------------+
    | Type 0x01: MSDOS Partition 12-bit FAT                     |
    +-----------------------------------------------------------+
    | Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
    +-----------------------------------------------------------+
    | Partition end   LCHS: Cylinder 0005, Head 063, Sector 63  |
    +-----------------------------------------------------------+
    | Sectors offset from MBR partition 0x0000003f              |
    +-----------------------------------------------------------+
    | Number of sectors in partition 0x00005e41                 |
    +-----------------------------------------------------------+
    | Sectors offset from start of disk 0x0000003f              |
    +-----------------------------------------------------------+

    +--------------------------------------+
    | Master Boot Record - Partition Table |
    +--------------------------------------+----------+
    | Current LCHS Cylinder 0000, Head 000, Sector 00 |
    +-----------------------------+-------------------+---------+
    | Partition Entry number 03   | Partition Entry offset 0x1ce|
    +-----------------------------+-----------------------------+
    | Status field = 0x00         | Non-bootable Partition      |
    +-----------------------------+-----------------------------+
    | Type 0x05: MSDOS Extended Partition                       |
    +-----------------------------------------------------------+
    | Partition start LCHS: Cylinder 0006, Head 000, Sector 01  |
    +-----------------------------------------------------------+
    | Partition end   LCHS: Cylinder 0014, Head 063, Sector 63  |
    +-----------------------------------------------------------+
    | Sectors offset from MBR partition 0x00005e80              |
    +-----------------------------------------------------------+
    | Number of sectors in partition 0x00200dc0                 |
    +-----------------------------------------------------------+
    | Sectors offset from start of disk 0x00005e80              |
    +-----------------------------------------------------------+

    +--------------------------------------+
    | Master Boot Record - Partition Table |
    +--------------------------------------+----------+
    | Current LCHS Cylinder 0000, Head 000, Sector 00 |
    +-----------------------------+-------------------+---------+
    | Partition Entry number 02   | Partition Entry offset 0x1de|
    +-----------------------------+-----------------------------+
    | Status field = 0x00         | Non-bootable Partition      |
    +-----------------------------+-----------------------------+
    | Type 0x00: Empty (NULL) Partition                         |
    +-----------------------------------------------------------+

    ....(REST of OUTPUT DELETED FOR SPACE).....
    
    There is going to be a lot more output than above.  The above 
    is simply an example of the format.  dosPartShowAta will display
    all four partition table entries for each partition sector.  
    For example, a disk with 4 logical drives (C,D,E, & F) will 
    contain 5 partition sectors and dosPartShowAta() will display
    20 partition table entries.  

    Utilities such as Norton Utilities DISKEDIT (TM) do not
    automaticly display extended partition data.  This must
    be done manually.  That is not a restriction with dosPartShowAta.
    dosPartShowAta will automatically display all extended partitions.



------------------------------------------------------------------------------
    ***********************************************************
3.) EXAMPLE OF MOUNTING DISK CONTAINING BOTH VXWORKS LONG FILE 
    NAME FILE SYSTEMS AND MSDOS 8.3 FILE NAME FILE SYSTEMS.
    ***********************************************************

    This code may be used to provide support for mounting disks
    simultaneously containing both VxWorks style "long names" (not
    MSDOS compatible) file systems and standard 8.3 MSDOS compatible
    file systems.  This is useful since the pc386 and pc486 boot loader
    (VxLD 1.2) will only boot from a 8.3 file system.  One could use
    FDISK.exe to create a small (10MB) primary partition for booting.
    Then create a larger extended partition.   Then, using this code to
    find the offset to the large extended partition, one would overlay
    a block device over the extended partition and format the extended
    partition with a long filename file system.  This will allow 
    pc486/pc486 boot from the 8.3 file name style 10MB partition and
    do data storage on the long-filename    partition.  VxWorks long 
    filename file systems are incompatible with MSDOS however.

    Here are the steps that work for me manually:
    (starting with a clean hard disk)


A.) Use FDISK.exe to create a 10MB primary boot partition.
    (Or however big you want the 8.3 boot partition to be)


B.) Use FDISK.exe to create a extended partition for the rest
    of the disk.  (or however big you want it to be)


C.) Use FDISK.exe to place a logical drive (or many) within
    the extended partition.  We will use this for the long 
    filename file system.


D.) Format the 10MB with MSDOS with "format c: /s /u /c" command.
    Leave the extended partition unformatted.


E.) Boot VxWorks and load dosPartLibAta.o


F.) View the partition data created in steps A-C:

    For Example:

    -> sp dosPartShowAta,0,0             
    task spawned: id = 1fb47b0, name = u0
    value = 33245104 = 0x1fb47b0
    -> 
    
    On my system this produces the following output:

    +--------------------------------------+
    | Master Boot Record - Partition Table |
    +--------------------------------------+----------+
    | Current LCHS Cylinder 0000, Head 000, Sector 00 |
    +-----------------------------+-------------------+---------+
    | Partition Entry number 04   | Partition Entry offset 0x1be|
    +-----------------------------+-----------------------------+
    | Status field = 0x80         | Primary (bootable) Partition|
    +-----------------------------+-----------------------------+
    | Type 0x01: MSDOS Partition 12-bit FAT                     |
    +-----------------------------------------------------------+
    | Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
    +-----------------------------------------------------------+
    | Partition end   LCHS: Cylinder 0005, Head 063, Sector 63  |
    +-----------------------------------------------------------+
    | Sectors offset from MBR partition 0x0000003f              |
    +-----------------------------------------------------------+
    | Number of sectors in partition 0x00005e41                 |
    +-----------------------------------------------------------+
    | Sectors offset from start of disk 0x0000003f              |
    +-----------------------------------------------------------+

    +--------------------------------------+
    | Master Boot Record - Partition Table |
    +--------------------------------------+----------+
    | Current LCHS Cylinder 0000, Head 000, Sector 00 |
    +-----------------------------+-------------------+---------+
    | Partition Entry number 03   | Partition Entry offset 0x1ce|
    +-----------------------------+-----------------------------+
    | Status field = 0x00         | Non-bootable Partition      |
    +-----------------------------+-----------------------------+
    | Type 0x05: MSDOS Extended Partition                       |
    +-----------------------------------------------------------+
    | Partition start LCHS: Cylinder 0006, Head 000, Sector 01  |
    +-----------------------------------------------------------+
    | Partition end   LCHS: Cylinder 0014, Head 063, Sector 63  |
    +-----------------------------------------------------------+
    | Sectors offset from MBR partition 0x00005e80              |
    +-----------------------------------------------------------+
    | Number of sectors in partition 0x00200dc0                 |
    +-----------------------------------------------------------+
    | Sectors offset from start of disk 0x00005e80              |
    +-----------------------------------------------------------+
    ....(All Empty partition data deleted for space.)...

    +-----------------------------+
    | Extended Partition Table 01 |
    +-----------------------------+-------------------+
    | Current LCHS Cylinder 0006, Head 000, Sector 01 |
    +-----------------------------+-------------------+---------+
    | Partition Entry number 04   | Partition Entry offset 0x1be|
    +-----------------------------+-----------------------------+
    | Status field = 0x00         | Non-bootable Partition      |
    +-----------------------------+-----------------------------+
    | Type 0x06: MSDOS 16-bit FAT >=32M Partition               |
    +-----------------------------------------------------------+
    | Partition start LCHS: Cylinder 0006, Head 001, Sector 01  |
    +-----------------------------------------------------------+
    | Partition end   LCHS: Cylinder 0014, Head 063, Sector 63  |
    +-----------------------------------------------------------+
    | Sectors offset from first extended partition 0x0000003f   |
    +-----------------------------------------------------------+
    | Number of sectors in partition 0x00200d81                 |
    +-----------------------------------------------------------+
    | Sectors offset from start of disk 0x00005ebf              |
    +-----------------------------------------------------------+
    ....(All Empty partition data deleted for space.)...


    Note the "Extended Partition Table 01" "Partition Entry number 04"
    values for "Number of sectors in partition 0x00200d81" and
    "Sectors offset from start of disk 0x00005ebf"  (Values will 
    vary for different disks, obviously)
G.) Using these values, create a block device from the shell:

    -> pMyAtaBlkDev = ataDevCreate (0,0, 0x00200d81, 0x00005ebf)         
    new symbol "pMyAtaBlkDev" added to symbol table.
    pMyAtaBlkDev = 0x36a860: value = 33245356 = 0x1fb48ac
    -> 

    AKA:
    ptrAtaBlkDev = ataDevCreate (ctrl, drive, blkSize, blkOffset);


H.) Using this block device, create a long filename file system
    upon it: (DOS_OPT_LONGNAMES == 0x4)

    -> dosFsMkfsOptionsSet (0x4)
    value = 0 = 0x0

    -> dosFsMkfs ("/ataCtrl0Drive0Part1", pMyAtaBlkDev)
    value = 33245200 = 0x1fb4810

    -> dosFsConfigShow "/ataCtrl0Drive0Part1"  

    device name:               /ataCtrl0Drive0Part1
    total number of sectors:   2100609
    bytes per sector:          512
    media byte:                0xf0
    # of sectors per cluster:  33
    # of reserved sectors:     1
    # of FAT tables:           2
    # of sectors per FAT:      249
    max # of root dir entries: 112
    # of hidden sectors:       0
    removable medium:          TRUE
    disk change w/out warning: not enabled
    auto-sync mode:            not enabled
    long file names:           ENABLED
    exportable file system:    not enabled
    lowercase-only filenames:  not enabled
    volume mode:               O_RDWR (read/write)
    available space:           1075244544 bytes
    max avail. contig space:   1075244544 bytes
    value = 0 = 0x0
    -> 

I.) Reset parameters to use ata booting in config.h:

    For example:

    #define DEFAULT_BOOT_LINE \
    "ata=0,0(0,0)eno:/ata0/vxWorks h=90.0.0.3 \ 
     e=90.0.0.50:ff000000 u=johnx pw=cogsNcogs! tn=t90-50 o=fei"

    #define SYS_WARM_TYPE      2 (0=BIOS, 1=Floppy, 2=Ata (Hard))
    #define SYS_WARM_ATA_CTRL  0 (0 = primary ctrl: 1= secondary ctrl)
    #define SYS_WARM_ATA_DRIVE 0 (0 = master :1 = slave)   
    
    Remake your bootrom/bootrom_uncmp & vxWorks images.


J.) Use vxsys.com to write the VxLD-1.2 boot sector loader to C:

    Note also: Contact WRS Customer Support for versions
    of vxsys/vxcopy utilities which function in MSDOS-OS.
    The versions shipped with tornado only work in Windows95/NT
    It is easier to use the MSDOS versions when making bootable
    hard disks.  Window95 primary partitions must be unlocked
    with the internal MSDOS command "C:\> lock c:"


K.) Use vxcopy to copy the bootrom image to the C: primary
    partition.  


L.) Use xcopy.exe to copy the vxWorks image to the root directory
    of the primary boot partition.

    Note that you will need to save the core file on the Tornado
    host and point the target server to it with the "-c" option
    when booting from an ATA disk when using the host tools.


M.) Once you've booted VxWorks from the 10MB 8.3 partition
    you may use this routine to mount both the 8.3 file system
    in the primary boot partition and the long filenames (40chars) 
    file system in the extended partition you created:

    -> sp dosPartMountAta,0,0                                   
    task spawned: id = 1fb47b0, name = u0
    value = 33245104 = 0x1fb47b0
    -> 
    Mounting partition 00 - VxWorks device Name "/ataCtrl0Drive0Part0"
     Absolute sector: 0x0000003f  Number of sectors: 0x00005e41

    Mounting partition 01 - VxWorks device Name "/ataCtrl0Drive0Part1"
     Absolute sector: 0x00005ebf  Number of sectors: 0x00200d81


    -> dosFsConfigShow "/ataCtrl0Drive0Part0"
    device name:               /ataCtrl0Drive0Part0
    total number of sectors:   24129
    bytes per sector:          512
    media byte:                0xf8
    # of sectors per cluster:  8
    # of reserved sectors:     1
    # of FAT tables:           2
    # of sectors per FAT:      9
    max # of root dir entries: 512
    # of hidden sectors:       63
    removable medium:          TRUE
    disk change w/out warning: not enabled
    auto-sync mode:            not enabled
    long file names:           not enabled
    exportable file system:    not enabled
    lowercase-only filenames:  not enabled
    volume mode:               O_RDWR (read/write)
    available space:           11599872 bytes
    max avail. contig space:   11599872 bytes
    value = 0 = 0x0

    -> dosFsConfigShow "/ataCtrl0Drive0Part1"

    device name:               /ataCtrl0Drive0Part1
    total number of sectors:   2100609
    bytes per sector:          512
    media byte:                0xf0
    # of sectors per cluster:  33
    # of reserved sectors:     1
    # of FAT tables:           2
    # of sectors per FAT:      249
    max # of root dir entries: 112
    # of hidden sectors:       0
    removable medium:          TRUE
    disk change w/out warning: not enabled
    auto-sync mode:            not enabled     See:
    long file names:           ENABLED      <- LONG FILENAMES!
    exportable file system:    not enabled
    lowercase-only filenames:  not enabled
    volume mode:               O_RDWR (read/write)
    available space:           1075244544 bytes
    max avail. contig space:   1075244544 bytes
    value = 0 = 0x0
    -> 

------------------------------------------------------------------------------

    ***********************************************************
4.)  EXAMPLE OF USING dosPartMountScsi and dosPartShowScsi
    ***********************************************************

VxWorks (for Motorola MVME167) version 5.3.1.

-> scsiAutoConfig     
value = 0 = 0x0

-> scsiShow
ID LUN VendorID    ProductID     Rev. Type  Blocks  BlkSize pScsiPhysDev 
-- --- -------- ---------------- ---- ---- -------- ------- ------------
 0  0  QUANTUM  P80S 980-80-94xx 6.8    0   164058    512    0x01fff7f8 
 4  0  CONNER   CFP1080S         4649   0  2110812    512    0x01ffe518 
value = 0 = 0x0

-> 

-> ld < C:\dosPartLib\scsi\dosPartLibScsi.o
value = 14385160 = 0xdb8008
-> 

-> sp dosPartMountScsi, pSysScsiCtrl
task spawned: id = 1fb5814, name = u0
value = 33249300 = 0x1fb5814
-> 

Mounting partition 00 - VxWorks device Name "ID0/LUN0/Part0"
Absolute sector: 0x00000020  Number of sectors: 0x00003fe0
Approximate Partition size: 7 MBs

Mounting partition 01 - VxWorks device Name "ID0/LUN0/Part1"
Absolute sector: 0x00004020  Number of sectors: 0x000047e0
Approximate Partition size: 8 MBs

Mounting partition 02 - VxWorks device Name "ID0/LUN0/Part2"
Absolute sector: 0x00008820  Number of sectors: 0x000067e0
Approximate Partition size: 12 MBs

Mounting partition 03 - VxWorks device Name "ID0/LUN0/Part3"
Absolute sector: 0x0000f020  Number of sectors: 0x000027e0
Approximate Partition size: 4 MBs

Mounting partition 04 - VxWorks device Name "ID0/LUN0/Part4"
Absolute sector: 0x00011820  Number of sectors: 0x00001fe0
Approximate Partition size: 3 MBs

Mounting partition 05 - VxWorks device Name "ID0/LUN0/Part5"
Absolute sector: 0x00013820  Number of sectors: 0x00010fe0
Approximate Partition size: 33 MBs

Mounting partition 06 - VxWorks device Name "ID0/LUN0/Part6"
Absolute sector: 0x00024820  Number of sectors: 0x00000fe0
Approximate Partition size: 1 MBs

Mounting partition 07 - VxWorks device Name "ID0/LUN0/Part7"
Absolute sector: 0x00025820  Number of sectors: 0x000027e0
Approximate Partition size: 4 MBs


Mounting partition 00 - VxWorks device Name "ID4/LUN0/Part0"
Absolute sector: 0x0000003f  Number of sectors: 0x00032f8e
Approximate Partition size: 101 MBs

Mounting partition 01 - VxWorks device Name "ID4/LUN0/Part1"
Absolute sector: 0x0003300c  Number of sectors: 0x0000fac5
Approximate Partition size: 31 MBs

Mounting partition 02 - VxWorks device Name "ID4/LUN0/Part2"
Absolute sector: 0x00042b10  Number of sectors: 0x00098f28
Approximate Partition size: 305 MBs

Mounting partition 03 - VxWorks device Name "ID4/LUN0/Part3"
Absolute sector: 0x000dba77  Number of sectors: 0x00065f5b
Approximate Partition size: 203 MBs

Mounting partition 04 - VxWorks device Name "ID4/LUN0/Part4"
Absolute sector: 0x00141a11  Number of sectors: 0x00065f5b
Approximate Partition size: 203 MBs


-> 
-> devs
drv name
  0 /null
  1 /tyCo/0
  1 /tyCo/1
  1 /tyCo/2
  1 /tyCo/3
  6 eno:
  7 /vio
  3 ID0/LUN0/Part0
  3 ID0/LUN0/Part1
  3 ID0/LUN0/Part2
  3 ID0/LUN0/Part3
  3 ID0/LUN0/Part4
  3 ID0/LUN0/Part5
  3 ID0/LUN0/Part6
  3 ID0/LUN0/Part7
  3 ID4/LUN0/Part0
  3 ID4/LUN0/Part1
  3 ID4/LUN0/Part2
  3 ID4/LUN0/Part3
  3 ID4/LUN0/Part4
value = 0 = 0x0

-> dosFsConfigShow "ID4/LUN0/Part3"

device name:               ID4/LUN0/Part3
total number of sectors:   417627
bytes per sector:          512
media byte:                0xf8
# of sectors per cluster:  8
# of reserved sectors:     1
# of FAT tables:           2
# of sectors per FAT:      204
max # of root dir entries: 512
# of hidden sectors:       63
removable medium:          false
disk change w/out warning: not enabled
auto-sync mode:            not enabled
long file names:           not enabled
exportable file system:    not enabled
lowercase-only filenames:  not enabled
volume mode:               O_RDWR (read/write)
available space:           213598208 bytes
max avail. contig space:   213598208 bytes
value = 0 = 0x0
-> 


-> sp dosPartShowScsi, 0x01fff7f8
task spawned: id = 1fb5814, name = u0
value = 33249300 = 0x1fb5814
-> 

+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+--------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x80         | Primary (bootable) Partition|
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from MBR partition 0x00000020              |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00003fe0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00000020              |
+-----------------------------------------------------------+



+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+--------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from MBR partition 0x00004000              |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00024000                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00004000              |
+-----------------------------------------------------------+



+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+--------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+



+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+--------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x000047e0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00004020              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00004800     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00006800                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00008800              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 02 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x000067e0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00008820              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 02 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x0000b000     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00002800                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00013800              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 02 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 02 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 03 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x000027e0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x0000f020              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 03 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x0000d800     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00002000                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x0001c800              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 03 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 03 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 04 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00001fe0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00011820              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 04 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x0000f800     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00011000                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00021000              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 04 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 04 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 05 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x06: MSDOS 16-bit FAT >=32M Partition               |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00010fe0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00013820              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 05 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00020800     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00001000                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00034000              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 05 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 05 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 06 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00000fe0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00024820              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 06 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00021800     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00002800                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00046000              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 06 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 06 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 07 |
+-----------------------------+-----------------------------+
| Partition Entry number 00   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0000, Head 063, Sector 32  |
+-----------------------------------------------------------+
| Sectors offset from the extended partition 0x00000020     |
+-----------------------------------------------------------+
| Number of sectors in partition 0x000027e0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x00025820              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 07 |
+-----------------------------+-----------------------------+
| Partition Entry number 01   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 07 |
+-----------------------------+-----------------------------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 07 |
+-----------------------------+-----------------------------+
| Partition Entry number 03   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


->

------------------------------------------------------------------------------

    ***********************************************************
5.) ANOTHER EXAMPLE OF BOOTING FROM AND MOUNTING DISK CONTAINING
    BOTH VXWORKS LONG FILE NAME FILE SYSTEMS AND MSDOS 8.3 FILE
    NAME FILE SYSTEMS.  ALSO SHOWS ADDING dosPartLib TO usrExtra.c
    ***********************************************************

-----------
1.)

With MSDOS version 4.00.95 (Windows 95 version)
I repartitioned the drive. It was a 776 MB drive. 

I created a primary bootable/active partition (C:) 10MB in size.   
I created an extended partition using the remaining 766 MB of space .
I assigned logical drive "D:" to use the entire extended partition, 766MB.

-----------
2.)

I formatted C: with the command "A:\> format c: /s/u/c"

-----------
3.)

I edited pc486 config.h to read:

#define DEFAULT_BOOT_LINE \
"ata=0,0(0,0)devhost:/ataCtrl0Drive0Part0/vxWorks.st \
 h=90.0.0.3 e=90.0.0.50 u=vxWorks pw=vxWorks o=pcmcia=1"

(pcmcia=1 is the elt device in the "top" pcmcia slot of the Toshiba )

#define INCLUDE_PCMCIA
#define INCLUDE_SHOW_ROUTINES
#define SYS_WARM_TYPE 2   

(0=BIOS, 1=Floppy, 2=Ata (Hard))

-----------
4.)

I rebuilt "bootrom" image and wrote it to the hard disk 
with vxsys.com & vxcopy.exe from version 5.2, which run 
under msdos (ie from my bootable MSDOS floppy).


A:\> lock c:      - Required for Win95 version of MSDOS.
A:\> vxsys c:    - Write VxLD to boot sector.
A:\> vxcopy bootrom c:\bootrom.sys  - copy vxWorks bootrom image to hdd.

-----------
5.)

I built dosPartLib.o from dosPartLibAta.c and dosPartLibAta.h
And added to Makefile:
MACH_EXTRA=dosPartLib.o   
I then built vxWorks and copied it to C:\ on the toshiba laptop.

-----------
6.)

I then added to config.h

#define INCLUDE_CONFIGURATION_5_2
#define STANDALONE_NET

And built vxWorks.st and copied it to C:\ on the toshiba.
I also built some other various image like "vxw_com1.96"
for serial console.

The idea here is that you can just change the boot parameters
to boot either vxWorks for use with tornado host tools, or vxWorks.st 
with 5.2 style tools, or over the pcmcia=1 (top) slot elt network device.

Also, look how fast reboot works from ata hard disk.
when SYS_WARM_TYPE  is defined as 2.

-----------
7.)

The toshiba now boots the "bootrom" image from ATA via vxLd 1.2....
The bootrom image would load vxWorks from the ata device and also
initialize the PCMCIA ELT3 network device during boot by defualt.
Boot parameter could be changed to boot over pcmcia or from 
floppy device.

-----------
8.)

Here is what the boot looked like:

On console:

Press any key to stop auto-boot...
 0
auto-booting...


boot device          : ata=0,0
processor number     : 0
host name            : devhost
file name            : /ataCtrl0Drive0Part0/vxWorks
inet on ethernet (e) : 90.0.0.50
host inet (h)        : 90.0.0.3
user (u)             : vxWorks
ftp password (pw)    : vxWorks
flags (f)            : 0x0
other (o)            : pcmcia=1

Attaching to ATA disk device... done.
Loading /ataCtrl0Drive0Part0/vxWorks...434728 + 10296 + 35016
Starting at 0x108000...

Target Name: vxTarget
Attaching network interface pcmcia0... done.
Attaching network interface lo0... done.
NFS client support not included.


                 VxWorks

Copyright 1984-1996  Wind River Systems, Inc.

            CPU: PC 486
        VxWorks: 5.3.1
    BSP version: 1.1/2
  Creation date: Aug 19 1997
            WDB: Ready.

On (host) target server window:

tgtsvr.exe 90.0.0.50 -n pc486rpc -c C:\Sirocco\target\config\pc486\vxWorks -V
tgtsvr.exe (pc486rpc@eno): Tue Aug 19 01:39:53 1997
    License request... authorized on Local Host.
    Wind River Systems Target Server: NT/Win95 version
    Attaching backend... succeeded.
    Connecting to target agent... succeeded.
    Attaching C++ interface... succeeded.
    Attaching a.out OMF reader... succeeded.

(I have to use a core file since I am booting vxWorks from ata.)

I then could start a WindShell and view the partition information
with dosPartShowAta() from my partition code...


      /////   /////   /////   /////   /////       |
     /////   /////   /////   /////   /////        |
    /////   /////   /////   /////   /////         |
    /////   /////   /////   /////   /////         |
   //////  //////  //////  //////  //////         |     
   //////  //////  //////  //////  //////         |     T  O  R  N  A  D  O 
    /////   /////   /////   /////   /////         |                        
    /////   /////   /////   /////   /////         |                        
     /////   /////   /////   /////   /////        |     Development  System
      ////    ////    ////    ////    ////        |                        
       ////    ////    ////    ////    ////       |                        
        ////    ////    ////    ////    ////      |     Host  Based   Shell
         ////    ////    ////    ////    ////     |                        
          ////    ////    ////    ////    ////    |                        
           ///     ///     ///     ///     ///    |     Version  1.0.1
           ///     ///     ///     ///     ///    |     
            //      //      //      //      //    |
            //      //      //      //      //    |
            //      //      //      //      //    |
           //      //      //      //      //     |

      Copyright Wind River Systems, Inc., 1995-1997

C++ Constructors/Destructors Strategy is AUTOMATIC

-> <C:\Startup.script
myFd = open ("/vio/0",2,0)
new symbol "myFd" added to symbol table.
myFd = 0x2eb8e4: value = 7 = 0x7
ioGlobalStdSet(0, myFd)
value = 0 = 0x0
ioGlobalStdSet(1, myFd)
value = 1 = 0x1
ioGlobalStdSet(2, myFd)
value = 2 = 0x2
logFdAdd (myFd)
value = 0 = 0x0
#logFdSet (myFd)

---------
14.) Now show the ATA HDD's partition data with dosPartShowAta (ctrl, drive)

-> dosPartShowAta 0,0

+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+----------+
| Current LCHS Cylinder 0000, Head 000, Sector 00 |
+-----------------------------+-------------------+---------+
| Partition Entry number 04   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x80         | Primary (bootable) Partition|
+-----------------------------+-----------------------------+
| Type 0x01: MSDOS Partition 12-bit FAT                     |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0000, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0010, Head 031, Sector 63  |
+-----------------------------------------------------------+
| Sectors offset from MBR partition 0x0000003f              |
+-----------------------------------------------------------+
| Number of sectors in partition 0x00005661                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x0000003f              |
+-----------------------------------------------------------+



+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+----------+
| Current LCHS Cylinder 0000, Head 000, Sector 00 |
+-----------------------------+-------------------+---------+
| Partition Entry number 03   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x05: MSDOS Extended Partition                       |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0011, Head 000, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0019, Head 031, Sector 63  |
+-----------------------------------------------------------+
| Sectors offset from MBR partition 0x000056a0              |
+-----------------------------------------------------------+
| Number of sectors in partition 0x0017e6e0                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x000056a0              |
+-----------------------------------------------------------+




+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+----------+
| Current LCHS Cylinder 0000, Head 000, Sector 00 |
+-----------------------------+-------------------+---------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+



+--------------------------------------+
| Master Boot Record - Partition Table |
+--------------------------------------+----------+
| Current LCHS Cylinder 0000, Head 000, Sector 00 |
+-----------------------------+-------------------+---------+
| Partition Entry number 01   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-------------------+
| Current LCHS Cylinder 0011, Head 000, Sector 01 |
+-----------------------------+-------------------+---------+
| Partition Entry number 04   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x06: MSDOS 16-bit FAT >=32M Partition               |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0011, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0019, Head 031, Sector 63  |
+-----------------------------------------------------------+
| Sectors offset from first extended partition 0x0000003f   |
+-----------------------------------------------------------+
| Number of sectors in partition 0x0017e6a1                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x000056df              |
+-----------------------------------------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-------------------+
| Current LCHS Cylinder 0011, Head 000, Sector 01 |
+-----------------------------+-------------------+---------+
| Partition Entry number 03   | Partition Entry offset 0x1ce|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-------------------+
| Current LCHS Cylinder 0011, Head 000, Sector 01 |
+-----------------------------+-------------------+---------+
| Partition Entry number 02   | Partition Entry offset 0x1de|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+


+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-------------------+
| Current LCHS Cylinder 0011, Head 000, Sector 01 |
+-----------------------------+-------------------+---------+
| Partition Entry number 01   | Partition Entry offset 0x1ee|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x00: Empty (NULL) Partition                         |
+-----------------------------+-----------------------------+

value = 0 = 0x0
-> 

-----------
15.)

Using these values I made a long file name file system
in the logical drive in the extended partition:

Here is the table used which shows the logical drive
in the extended partition (from output above):

+-----------------------------+
| Extended Partition Table 01 |
+-----------------------------+-------------------+
| Current LCHS Cylinder 0011, Head 000, Sector 01 |
+-----------------------------+-------------------+---------+
| Partition Entry number 04   | Partition Entry offset 0x1be|
+-----------------------------+-----------------------------+
| Status field = 0x00         | Non-bootable Partition      |
+-----------------------------+-----------------------------+
| Type 0x06: MSDOS 16-bit FAT >=32M Partition               |
+-----------------------------+-----------------------------+
| Partition start LCHS: Cylinder 0011, Head 001, Sector 01  |
+-----------------------------------------------------------+
| Partition end   LCHS: Cylinder 0019, Head 031, Sector 63  |
+-----------------------------------------------------------+
| Sectors offset from first extended partition 0x0000003f   |
+-----------------------------------------------------------+
| Number of sectors in partition 0x0017e6a1                 |
+-----------------------------------------------------------+
| Sectors offset from start of disk 0x000056df              |
+-----------------------------------------------------------+


The value for "Number of sectors in partition 0x0017e6a1"
and "Sectors offset from start of disk 0x000056df" are what
is needfed to overlay a block device upon the partition:

For example:

-> pMyAtaBlkDev = ataDevCreate (0,0, 0x0017e6a1, 0x000056df)
new symbol "pMyAtaBlkDev" added to symbol table.
pMyAtaBlkDev = 0x2eb8dc: value = 25356388 = 0x182e864
-> 

-> dosFsMkfsOptionsSet (0x4)   (0x04 == LONG FILENAMES)
value = 0 = 0x0
->

-> dosFsMkfs ("/ataCtrl0Drive0Part1", pMyAtaBlkDev)
value = 25356232 = 0x182e7c8

-> dosFsConfigShow "/ataCtrl0Drive0Part1"  
device name:               /ataCtrl0Drive0Part1
total number of sectors:   1566369
bytes per sector:          512
media byte:                0xf0
# of sectors per cluster:  24
# of reserved sectors:     1
# of FAT tables:           2
# of sectors per FAT:      255
max # of root dir entries: 112
# of hidden sectors:       0
removable medium:          TRUE
disk change w/out warning: not enabled
auto-sync mode:            not enabled
long file names:           ENABLED
exportable file system:    not enabled
lowercase-only filenames:  not enabled
volume mode:               O_RDWR (read/write)
available space:           801705984 bytes
max avail. contig space:   801705984 bytes
value = 0 = 0x0
-> 

-> myFd = creat ("/ataCtrl0Drive0Part1/longFileNameFileSystem.tmp",2,0)
myFd = 0x2eb8e4: value = 8 = 0x8
->

-----------

Then I reboot the target to show the mounting of both
the short file name file system and the long filename file
system.  (reboot works from ATA since I changed the 
SYS_WARM_TYPE to 2 in config.h)

-> reboot
Rebooting...

Press any key to stop auto-boot...
 0
auto-booting...


boot device          : ata=0,0
processor number     : 0
host name            : devhost
file name            : /ataCtrl0Drive0Part0/vxWorks
inet on ethernet (e) : 90.0.0.50
host inet (h)        : 90.0.0.3
user (u)             : vxWorks
ftp password (pw)    : vxWorks
flags (f)            : 0x0
other (o)            : pcmcia=1

Attaching to ATA disk device... done.
Loading /ataCtrl0Drive0Part0/vxWorks...434728 + 10296 + 35016
Starting at 0x108000...

Target Name: vxTarget
Attaching network interface pcmcia0... done.
Attaching network interface lo0... done.
NFS client support not included.

                 VxWorks

Copyright 1984-1996  Wind River Systems, Inc.

            CPU: PC 486
        VxWorks: 5.3.1
    BSP version: 1.1/2
  Creation date: Aug 19 1997
            WDB: Ready.


      /////   /////   /////   /////   /////       |
     /////   /////   /////   /////   /////        |
    /////   /////   /////   /////   /////         |
    /////   /////   /////   /////   /////         |
   //////  //////  //////  //////  //////         |     
   //////  //////  //////  //////  //////         |     T  O  R  N  A  D  O 
    /////   /////   /////   /////   /////         |                        
    /////   /////   /////   /////   /////         |                        
     /////   /////   /////   /////   /////        |     Development  System
      ////    ////    ////    ////    ////        |                        
       ////    ////    ////    ////    ////       |                        
        ////    ////    ////    ////    ////      |     Host  Based   Shell
         ////    ////    ////    ////    ////     |                        
          ////    ////    ////    ////    ////    |                        
           ///     ///     ///     ///     ///    |     Version  1.0.1
           ///     ///     ///     ///     ///    |     
            //      //      //      //      //    |
            //      //      //      //      //    |
            //      //      //      //      //    |
           //      //      //      //      //     |

      Copyright Wind River Systems, Inc., 1995-1997

C++ Constructors/Destructors Strategy is AUTOMATIC

> <C:\Startup.script                                                  
myFd = open ("/vio/0",2,0)
new symbol "myFd" added to symbol table.
myFd = 0x2eb8e4: value = 7 = 0x7
ioGlobalStdSet(0, myFd)
value = 0 = 0x0
ioGlobalStdSet(1, myFd)
value = 1 = 0x1
ioGlobalStdSet(2, myFd)
value = 2 = 0x2
logFdAdd (myFd)
value = 0 = 0x0
#logFdSet (myFd)
-> 


Load some tools  (dosPartLibAta.o is linked into image):

-> ld < C:\dosPartLib\targetIo.o                                       
value = 14364680 = 0xdb3008



-----------

Now mount all existing partitions with dosPartMountAta (ctrl, drive):

-> sp dosPartMountAta,0,0                                              
task spawned: id = 17fe674, name = u0
value = 25159284 = 0x17fe674
-> 

Mounting partition 00 - VxWorks device Name "/ataCtrl0Drive0Part0"
Absolute sector: 0x0000003f  Number of sectors: 0x00005661
Approximate Partition size: 10 MBs

Mounting partition 01 - VxWorks device Name "/ataCtrl0Drive0Part1"
Absolute sector: 0x000056df  Number of sectors: 0x0017e6a1
Approximate Partition size: 764 MBs




-----------

Now display the primary partition (C: to MSDOS)

-> 
-> cdTarget "/ataCtrl0Drive0Part0"
value = 0 = 0x0
-> lsTarget ".",1
  size          date       time       name
--------       ------     ------    --------
  223148    JUL-11-1995  09:50:00   IO.SYS            
       9    JUL-11-1995  09:50:00   MSDOS.SYS         
   92870    JUL-11-1995  09:50:00   COMMAND.COM       
   71287    JUL-11-1995  09:50:00   DRVSPACE.BIN      
  731228    AUG-19-1997  01:31:58   VXWORKS.ST        
       0    AUG-21-1997  13:45:40   BOOTLOG.TXT       
  161018    AUG-21-1997  13:50:54   BOOTROM.SYS       
  570501    AUG-19-1997  01:11:40   VXWORKS           
  500400    AUG-19-1997  01:02:40   VXW_COM1.96           

value = 0 = 0x0
-> 


(The long file name file system)

-> cdTarget "/ataCtrl0Drive0Part1"
value = 0 = 0x0

-> lsTarget ".",1                 

  size          date       time       name
--------       ------     ------    --------
       0    JAN-01-1980  00:00:00   longFileNameFileSystem.tmp  

value = 0 = 0x0

-> 


Whola!  Now you can place all that long file name data
file (java, web server ect.) and boot from the same hard
drive on the laptop.

-----------

Once it was setup, I modified VxWorks to mount automaticly at startup.  
For example.

1.) Copy dosPartLibAta.c to WIND_BASE/target/src/config


2.) Copy dosPartLibAta.h to WIND_BASE/target/h


3.) Add to WIND_BASE/target/config/pc486/config.h:


#define INCLUDE_DOS_PART_MOUNT_ATA


4.) Add to WIND_BASE/target/config/all/usrConfig.c:

#ifdef INCLUDE_DOS_PART_MOUNT_ATA
    if (taskSpawn ("dosMount", 20, 0, 40000, dosPartMountAta,
                   0,0,0,0,0,0,0,0,0,0) == ERROR)
        {
        printf ("usrConfig.c dosPartMountAta error.\n");
        }
#endif 



Add the above right before the comment:

"initialize the WDB debug agent"



5.) Add to WIND_BASE/target/src/config/usrExtra.c

#if     defined (INCLUDE_DOS_PART_MOUNT_ATA)
#include "../../src/config/dosPartLibAta.c"
#endif


Add it right before the: 
#ifdef  INCLUDE_DELETE_5_0


6.) Rebuild.  It will try mount all drive zero, ctrl zero's partitions.


The output of such a boot:

Target Name: vxTarget
Attaching network interface pcmcia0... done.
Attaching network interface lo0... done.
NFS client support not included.


                 VxWorks

Copyright 1984-1996  Wind River Systems, Inc.

            CPU: PC 486
        VxWorks: 5.3.1
    BSP version: 1.1/2
  Creation date: Aug 19 1997
            WDB: Ready.


Mounting partition 00 - VxWorks device Name "/ataCtrl0Drive0Part0"
Absolute sector: 0x0000003f  Number of sectors: 0x00005661
Approximate Partition size: 10 MBs

Mounting partition 01 - VxWorks device Name "/ataCtrl0Drive0Part1"
Absolute sector: 0x000056df  Number of sectors: 0x0017e6a1
Approximate Partition size: 764 MBs

(END OF USAGE EXAMPLES)

------------------------------------------------------------------------------

*/


/* includes */

#include "vxWorks.h"            /* VxWorks header files. /target/h  */
#include "stdio.h"              /* standard IO library (vxWorks)    */
#include "stdlib.h"             /* standard library (vxWorks)       */
#include "string.h"             /* string library (vxWorks)         */
#include "dosPartLibScsi.h"     /* This libraries header file.      */


/* imports */


/* defines */
#define PART_SIG_ADRS           0x1fe   /* dos partition signature  */
#define PART_SIG_MSB            0x55    /* msb of the partition sig */
#define PART_SIG_LSB            0xaa    /* lsb of the partition sig */
#define BYTES_PER_SECTOR        0x200   /* 512 bytes per sector     */
#define PART_IS_BOOTABLE        0x80    /* a dos bootable partition */
#define PART_NOT_BOOTABLE       0x00    /* not a bootable partition */
#define PART_TYPE_DOS4          0x06    /* dos 16b FAT, 32b secnum  */
#define PART_TYPE_DOSEXT        0x05    /* msdos extended partition */
#define PART_TYPE_DOS3          0x04    /* dos 16b FAT, 16b secnum  */
#define PART_TYPE_DOS12         0x01    /* dos 12b FAT, 32b secnum  */
#define PART_TYPE_WIN95_D4      0x0e    /* Win95 dosfs  16bf 32bs   */
#define PART_TYPE_WIN95_EXT     0x0f    /* Win95 extended partition */
#define NUM_SCSI_ID             0x06    /* Max SCSI ID's            */
#define NUM_SCSI_LUN            0x06    /* Max SCSI LUN's           */

#define BOOT_TYPE_OFFSET    0x0   /* boot type                      */
#define STARTSEC_HD_OFFSET  0x1   /* beginning sector head value    */
#define STARTSEC_SEC_OFFSET 0x2   /* beginning sector               */
#define STARTSEC_CYL_OFFSET 0x3   /* beginning cylinder             */
#define SYSTYPE_OFFSET      0x4   /* system indicator               */
#define ENDSEC_HD_OFFSET    0x5   /* ending sector head value       */
#define ENDSEC_SEC_OFFSET   0x6   /* ending sector                  */
#define ENDSEC_CYL_OFFSET   0x7   /* ending cylinder                */
#define NSECTORS_OFFSET     0x8   /* sector offset from reference   */
#define NSECTORS_TOTAL      0xc   /* number of sectors in part      */


/* Byte swapping macro's */

#if (_BYTE_ORDER == _BIG_ENDIAN)  /* swapping macro for big endian  */

#define	SWABL(pSource32, pDest32) \
             {swab ((char *) pSource32 + 2, (char *) pDest32, 2); \
              swab ((char *) pSource32, (char *) pDest32 + 2, 2);}

#if (_DYNAMIC_BUS_SIZING == FALSE)

#define	USWABL(pSource32, pDest32) \
             {uswab ((char *) pSource32 + 2, (char *) pDest32, 2); \
              uswab ((char *) pSource32, (char *) pDest32 + 2, 2);}

#else   /* _DYNAMIC_BUS_SIZING == TRUE */

#define	USWABL(pSource32, pDest32) SWABL(pSource32, pDest32)

#endif  /* _DYNAMIC_BUS_SIZING == TRUE */

#else	/* _BYTE_ORDER == _BIG_ENDIAN - little endian swapping     */

#define SWABL(pSource32, pDest32)       (*pDest32 = *pSource32)
#define	USWABL(pSource32, pDest32) 	SWABL(pSource32, pDest32)

#endif	/* _BYTE_ORDER == _BIG_ENDIAN */


/* typedefs */


/* statics */

LOCAL int extPartLevel = 0;  /* static for extended partitions counting  */

/* globals */


/* function declarations */

STATUS dosPartParseScsi              /* fills list with valid parts      */
    (
    SCSI_PHYS_DEV * pScsiPhysDev,    /* ptr to a SCSI physical device [] */
    LIST * pVxPartDll                /* ptr to dll for valid partitions  */
    );

STATUS dosPartRdLbaScsi
    (
    SCSI_PHYS_DEV *pScsiPhysDev,     /* ptr to a SCSI physical device [] */
    ULONG lbaSec,                    /* lba sector to read data from.    */
    char * secBuf                    /* ptr to data secBuf for MBR data  */
    );


/*****************************************************************************
*
* dosPartMountScsi - attempts to mounts all partitions on all ID's/LUN's.
*
* This routine loops through all SCSI ID's and LUN's attempting
* to mount all partitions with msdos file systems on every valid
* SCSI physical device found.
*
* This routine is intended to be user callable.
*
* This routine first creates a linked list appropriate for storing 
* partition nodes.  The routine calloc's memory for the linked list
* (LIST * pVxPartDll) of valid partitions.  It then initializes the
* list with lstInit().  List nodes are of type PART_NODE *.  It then
* calls dosPartParseScsi() passing a pointer to the new list.  When
* dosPartParseScsi returns, it has filled the list with any partition
* node pointers considered mountable by dosFsDevInit(). Then, the node
* data stored in the list is used to  create independent blockDevices
* which directly overlay the drives partitions.  (VxWorks block devices
* are created using a drivers block device creation routine, for SCSI 
* it is scsiBlkDevCreate.).  This routine then attempts to mount each
* of the blockDevices with dosFsDevInit(). Finally, we clean up with
* lstFree() & free().
* 
* This code must be spawned with a adequate stack size since 
* dosPartParseScsi() is recursive (recursion uses stack)
* and dosFsDevInit also uses some stack.
*
* For example:  
*
* -> sp dosPartMountScsi,pSysScsiCtrl
*    or
* -> taskSpawn ("pMount", 20, 0, 30000, dosPartMountScsi,pSysScsiCtrl)
*
* Do not call the routine directly from the shell, ala:
*
* -> dosPartMountScsi (pSysScsiCtrl)
* 
* Or else you will likely overflow stack space. 
* 
* RETURNS: OK or ERROR
*
*/

STATUS dosPartMountScsi     /* mounts partitions on all ID/LUN  */
    (
    SCSI_CTRL * pSysScsiCtrl       /* SCSI_CTRL pointer */
    )
    {
    LIST * pVxPartDll;      /* ptr to dll of mountable parts    */
    PART_NODE * pPartNode;  /* ptr to dll of valid partitions   */
                            /* see dosPartLibScsi.h for typedef */
    SCSI_PHYS_DEV *pScsiPhysDev;  /* ptr to a SCSI physdev []   */
    BLK_DEV * pBlkDev;      /* BLK_DEV ptr for SCSI device.     */
    int i;                  /* for loop through secs partitions */
    char devName[20];       /* device name for DOS file system  */
    int Id;                 /* scsi id */
    int Lun;                /* scsi lun */

    /* loop through all SCSI IDs. */

    for ( Id = 0;  Id <= NUM_SCSI_ID;  Id++ )
        {

        /* loop through all LUNs.   */

        for ( Lun = 0;  Lun <= NUM_SCSI_LUN;  Lun++ )
            {
            
            /* get physDevId for ID/LUN combination */
            pScsiPhysDev = scsiPhysDevIdGet (pSysScsiCtrl, 
                                             Id, 
                                             Lun);

            /* skip NULL physical devices */

            if ( pScsiPhysDev == NULL ) continue;

            /* calloc memory for a partition dll */

            if ((pVxPartDll = (LIST *) calloc ((sizeof (LIST)), 1)) == NULL)
                {
                printErr ("dosPartMountScsi error. calloc failed.\n");
                return (ERROR);         
                } 

            /* initialize the dll, (lstLib) */

            lstInit (pVxPartDll);

            /* parse the boot partition and fill dll with valid partitions */

            if  ((dosPartParseScsi (pScsiPhysDev, pVxPartDll)) == ERROR)
                {
                printErr ("dosPartMountScsi error. " 
                          "dosPartParseScsi returned error.\n");
                return (ERROR);
                }
    
            /* get a pointer to the first node in the dll (lstLib) */

            if ((pPartNode = (PART_NODE *) lstFirst (pVxPartDll))
                 == NULL)
                {
                printErr ("dosPartMountScsi error:"
                          " lstFirst returned NULL.\n");
                return (ERROR);
                }

            printf ("\n"); /* just for vanity */

            /* mount all the partitions in the list. */

            for (i=0; i < pVxPartDll->count; i++)
                {

                /* Create a unique device name for file system/partition */

                sprintf (devName, "ID%d/LUN%d/Part%d", Id, Lun, i);


                /* Create a block device for the partition */

                if ((pBlkDev = scsiBlkDevCreate 
                                    (pScsiPhysDev,
                                     pPartNode->dosPartNumBlks,
                                     pPartNode->dosPartBlkOffset))
                     == (BLK_DEV *) NULL)
                    {
                    printErr ("dosPartMountScsi error. " 
                              "error during scsiBlkDevCreate.\n");
                    return (ERROR);         
                    }


                /* Mount the file system on the block device */

                printf ("Mounting partition %02d - VxWorks"
                        " device Name \"", i);
                printf (devName);
                printf ("\"\n");
                printf ("Absolute sector: 0x%08lx  ",
                         pPartNode->dosPartBlkOffset);
                printf ("Number of sectors: 0x%08lx\n",
                         pPartNode->dosPartNumBlks);
                printf ("Approximate Partition size: %ld MBs\n\n",
                         pPartNode->dosPartNumBlks * 
                         BYTES_PER_SECTOR / 0x00100000);

                if (dosFsDevInit(devName, pBlkDev, NULL) == 
                    (DOS_VOL_DESC *) NULL)
                    {

                    /* If error, only warn user and continue */

                    printErr ("dosPartMountScsi error. " 
                              "Error during dosFsDevInit.\n");
                    printErr ("Error occurred attempting to mount"
                              " partition %02d.\n", i);
                    printErr ("Absolute sector: 0x%08lx  ",
                               pPartNode->dosPartBlkOffset);
                    printErr ("Number of sectors: 0x%08lx\n",
                               pPartNode->dosPartNumBlks);
                    printErr ("Possible unformatted partition or ");
                    printErr ("possible bad partition data.\n");
                    printErr ("Possible device name conflict.\n");
                    printErr ("Possible dosPartMountScsi has already "
                              "been called for this drive.\n");
                    printErr ("User warned.  Attempting to continue."
                              " (unless last partition in dll)\n");
                    }

                /* get the ptr to next valid node in the partition list */      

                pPartNode = (PART_NODE *) lstNext ((NODE *) pPartNode); 

                }
            /* recycle */

            lstFree (pVxPartDll); 
            free ((char *) pVxPartDll); 
            }
        }
    return (OK);
    }



/*****************************************************************************
*
* dosPartParseScsi - parse partitions on given drive/ctrl.
*
* This routine is not intended to be user callable.
* 
* This routine parses all existing partition tables on a disk.
* It adds partition node data entries to a dll which it has been 
* passed (LIST * pVxPartDll) for any partition which contains a
* file system mountable by dosFsDevInit().  The size (in sectors)
* of the partition and the absolute offset (in sectors) from the
* start of the drive are also stored, since that is what VxWorks
* block device create routines need to overlay partitions. 
* 
* The partition table must appear to be valid when checked with
* 0x55aa signature.   The partition table in the Master Boot Record
* will be parsed first.  If there are any extended partitions found,
* a (recursive) call to dosPartParseScsi is made to parse the extended
* partition(s) in the new sector(s).  This may end up using a few
* thousand bytes of stack if you have large numbers of partitions,
* so be aware.
*
* RETURNS: OK or ERROR.
*
*/

STATUS dosPartParseScsi      /* fills list with valid parts  */
    (
    SCSI_PHYS_DEV *pScsiPhysDev,     /* ptr to a SCSI physical device [] */
    LIST * pVxPartDll                /* ptr to dll for valid partitions  */
    )
     
    {
    UCHAR * secBuf;             /* a secBuf for the sector data */
    PART_NODE * pPartNode;      /* dll ptr of valid partitions  */
                                /* see dosPartLibScsi.h typedef */
    int i;                      /* used for loop through parts  */
    LOCAL int partOffset;       /* offset from partition tbl ent*/
    LOCAL ULONG extPartOffset;  /* for first ext part offset    */
    LOCAL ULONG currentOffset;  /* for current ext part offset  */ 

    /* if we are in MBR partition setup for a read to the 1st sector */

    if (extPartLevel == 0x0)
        {
        extPartOffset = 0x0;    /* Used to store Ext part offset */
        currentOffset = 0x0;    /* Used for partition offset     */
        }


    /* allocate a local secBuf for the read sectors MBR/Part data */

    if ((secBuf = malloc (BYTES_PER_SECTOR)) == NULL)
        {
        printErr ("dosPartParseScsi error. " 
                  "Error during malloc'ing sector secBuf.\n");
        return (ERROR);
        }


    /* read the partition sector (LBA format) into a secBuf */

    if ((dosPartRdLbaScsi (pScsiPhysDev, currentOffset, secBuf)) 
         == ERROR)
        {
        printErr ("dosPartParseScsi error. " 
                  "dosPartRdLbaScsi returned error.\n");
        return (ERROR);
        }


    /* Check the validity of partition data with 0x55aa signature */

    if ((secBuf[PART_SIG_ADRS] != PART_SIG_MSB) || 
        (secBuf[PART_SIG_ADRS+1] != PART_SIG_LSB))
        {
        printErr ("dosPartParseScsi error. " 
                  "Sector secBuf contains an invalid"
                  " MSDOS partition signature 0x55aa.\n");
        return (ERROR);
        }


    /*
     * Loop through all parts add mountable parts to dll, will call
     * itself recursively to parse extended partitions & logical
     * drives in extended parts.  Recursive functions may use a lot
     * of stack space. Developer should beware of stack overflow.
     */

    for (i=0 ; i<4 ; i++)
        {

	/* partition offset relative to DOS_BOOT_PART_TBL */

	partOffset = i * (sizeof (DOS_PART_TBL));


        /* continue to next loop iteration if an empty partition */

        if ((secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]) 
            == 0)
            {
            continue;
            }

        /* if it is a bootable & non-extended partition, add first */

        if (((secBuf[DOS_BOOT_PART_TBL + partOffset + BOOT_TYPE_OFFSET]
              == PART_IS_BOOTABLE) &&
             (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
              != PART_TYPE_DOSEXT)  &&
             (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
              != PART_TYPE_WIN95_EXT))  &&      
            ((secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
              == PART_TYPE_DOS4) ||
             (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
              == PART_TYPE_WIN95_D4) ||
             (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
              == PART_TYPE_DOS3) ||
             (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
              == PART_TYPE_DOS12)))
            {

            /* calloc memory for a node, all nodes are free'd in */
            /* calling routines lstFree() call. */

            if ((pPartNode = (PART_NODE *) calloc 
                ((sizeof (PART_NODE)), 1)) == NULL)
                {
                printErr ("dosPartParseScsi error. " 
                          "calloc failed for boot partition node.\n");
                return (ERROR);         
                } 

            /* Fill the node with the current partition tables data */ 

            pPartNode->dosPart.dospt_status =
             secBuf[DOS_BOOT_PART_TBL + partOffset + BOOT_TYPE_OFFSET];

            pPartNode->dosPart.dospt_startHead =
             secBuf[DOS_BOOT_PART_TBL + partOffset + STARTSEC_HD_OFFSET];

            pPartNode->dosPart.dospt_startSec =
             secBuf[DOS_BOOT_PART_TBL + partOffset + STARTSEC_SEC_OFFSET];

            pPartNode->dosPart.dospt_type =
             secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET];

            pPartNode->dosPart.dospt_endHead =
             secBuf[DOS_BOOT_PART_TBL + partOffset + ENDSEC_HD_OFFSET];

            pPartNode->dosPart.dospt_endSec = 
             secBuf[DOS_BOOT_PART_TBL + partOffset + ENDSEC_SEC_OFFSET];

	    /* Swap for proper endian/sizing */

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset
                     + NSECTORS_OFFSET]),
                   (&pPartNode->dosPart.dospt_absSec));

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                     + NSECTORS_TOTAL]),
                   (&pPartNode->dosPart.dospt_nSectors));

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                     + NSECTORS_OFFSET]),
                   (&pPartNode->dosPartBlkOffset));

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                     + NSECTORS_TOTAL]),
                   (&pPartNode->dosPartNumBlks));


            /* Add boot part as the first node to the dll */

            lstInsert (pVxPartDll, NULL, (NODE *) pPartNode);
            }


        /* Add non-boot partitions with DOS filesystems to end of list */

        else if ((secBuf[DOS_BOOT_PART_TBL + partOffset + BOOT_TYPE_OFFSET]
                  == PART_NOT_BOOTABLE) &&
                 ((secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
                   == PART_TYPE_DOS4) ||
                  (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
                   == PART_TYPE_DOS3) ||
                  (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
                   == PART_TYPE_WIN95_D4) ||
                  (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
                   == PART_TYPE_DOS12)))
            {

            /* calloc memory for a node (all are free'd in lstFree) */

            if ((pPartNode = (PART_NODE *) calloc 
                   ((sizeof (PART_NODE)), 1)) == NULL)
                {
                printErr ("dosPartParseScsi error. " 
                          "calloc failed for partition node.\n");
                return (ERROR);         
                } 


            /* Fill the node with the current partition tables data */ 

            pPartNode->dosPart.dospt_status =
             secBuf[DOS_BOOT_PART_TBL + partOffset + BOOT_TYPE_OFFSET];
            pPartNode->dosPart.dospt_startHead =
             secBuf[DOS_BOOT_PART_TBL + partOffset + STARTSEC_HD_OFFSET];
            pPartNode->dosPart.dospt_startSec =
             secBuf[DOS_BOOT_PART_TBL + partOffset + STARTSEC_SEC_OFFSET];
            pPartNode->dosPart.dospt_type = 
             secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET];
            pPartNode->dosPart.dospt_endHead =
             secBuf[DOS_BOOT_PART_TBL + partOffset + ENDSEC_HD_OFFSET];
            pPartNode->dosPart.dospt_endSec = 
             secBuf[DOS_BOOT_PART_TBL + partOffset + ENDSEC_SEC_OFFSET];


	    /* Swap for proper endian/sizing */

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset
                     + NSECTORS_OFFSET]),
                    (&pPartNode->dosPart.dospt_absSec));

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                     + NSECTORS_TOTAL]),
                    (&pPartNode->dosPart.dospt_nSectors));

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                     + NSECTORS_OFFSET]),
                    (&pPartNode->dosPartBlkOffset));

            USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                     + NSECTORS_TOTAL]),
                    (&pPartNode->dosPartNumBlks));

            pPartNode->dosPartBlkOffset += currentOffset;

            /* Add this node to the end of the dll */

            lstAdd (pVxPartDll, (NODE *) pPartNode); 
            }


        /* found an extended partition, call ourself to parse. */

        if ((secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
             == PART_TYPE_DOSEXT) ||
            (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
             == PART_TYPE_WIN95_EXT))
            {

            /*
             * store the offset for first ext partition or update the  
             * offset for a logical drive within the extended partition
             * All logical drives in the disks first extended partition
             * are referenced via other extended partitions within the
             * first. Offsets of logical drives are relative to the
             * first extended partitions offset.
             */


           if (extPartLevel > 0)
                {
                USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset
                         + NSECTORS_OFFSET]),
                        (&currentOffset));
                currentOffset += extPartOffset;
                }
 
           else  /* in MBR, setup first extended partition offset */
                {
                USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset
                         + NSECTORS_OFFSET]),
                        (&extPartOffset));
                currentOffset = extPartOffset;
                }

            /* Update extPartLevel and parse found extended partition */

            extPartLevel++;
            dosPartParseScsi (pScsiPhysDev, pVxPartDll);
            extPartLevel--;
            }
        /* loop here to next part or end looping */
        }

    /* Clean up */

    free (secBuf);  
    return (OK);
    }




/*****************************************************************************
*
* dosPartRdLbaScsi - Read a Logical Block Address from a SCSI device
*
* This routine will contstruct a SCSI_COMMAND to read a sector.
* 
* RETURNS OK or ERROR.
*
*/


STATUS dosPartRdLbaScsi
    (
    SCSI_PHYS_DEV *pScsiPhysDev,     /* ptr to a SCSI physical device [] */
    ULONG lbaSec,                    /* lba sector to read data from.    */
    char * secBuf                    /* ptr to data secBuf for MBR data  */
    )

    {
    SCSI_COMMAND myScsiCmdBlock;    /* SCSI command byte array    */
    SCSI_TRANSACTION myScsiXaction; /* info for SCSI transaction  */

    /* build a scsi command to read a sector */
    

    if (lbaSec <= 0x1FFFFF)
        {
        /* build a 21 bit logical block address 'O_RDONLY' command */

	if (scsiCmdBuild (myScsiCmdBlock, &myScsiXaction.cmdLength,
			  SCSI_OPCODE_READ, pScsiPhysDev->scsiDevLUN, 
                          FALSE, lbaSec, 1, (UINT8) 0) == ERROR)
	    return (ERROR);
        }
    else	/* 01d fix - check for 21 bit LBA, else use EXT read. */
        {
        /* build a 32 bit logical block address 'READ_EXTENDED' command */

	if (scsiCmdBuild (myScsiCmdBlock, &myScsiXaction.cmdLength,
			  SCSI_OPCODE_READ_EXT, pScsiPhysDev->scsiDevLUN, 
			  FALSE, lbaSec, 1, (UINT8) 0) == ERROR)
	    return (ERROR);
        }



    /* fill in fields of SCSI_TRANSACTION structure */

    myScsiXaction.cmdAddress    = myScsiCmdBlock;
    myScsiXaction.dataAddress   = (UINT8 *) secBuf;
    myScsiXaction.dataDirection = O_RDONLY;
    myScsiXaction.dataLength    = pScsiPhysDev->blockSize; 
    myScsiXaction.cmdTimeout    = (SCSI_TIMEOUT_5SEC * 500);
    myScsiXaction.addLengthByte = NONE;
    myScsiXaction.tagType       = SCSI_TAG_DEFAULT;
    myScsiXaction.priority      = 50;

    if (scsiIoctl (pScsiPhysDev, FIOSCSICOMMAND, (int) &myScsiXaction) 
        == ERROR)
        {
        printErr ("dosPartRdLbaScsi error: scsiIoctl returned error.\n");
        return (ERROR);
        }
    return (OK);
    }


/* SCSI Dos partition show routines below.  This is used for     */
/* viewing the partition tables for debugging or informative    */
/* purposes. Once all is working well on a particular system,   */
/* INCLUDE_PART_SHOW should be undefined to reduce the size.    */


#ifdef INCLUDE_PART_SHOW

/*****************************************************************************
*
* dosPartShowScsi - parse and display partition data for a drive/ctrl.
*
* This routine is intended to be user callable.
*
* A device dependent partition table show routine.  This 
* routine outputs formatted data for all partition table 
* fields for every partition table found on a given disk,
* starting with the MBR sectors partition table.  It calls 
* dosPartLbaScsi() to read the sectors.  This code can be 
* removed to reduce code size by undefining: INCLUDE_PART_SHOW.
* Developers may use size<arch> to view code size.
*
* RETURNS: OK or ERROR
*
*/

STATUS dosPartShowScsi       /* show all partitions on device */
    (
    SCSI_PHYS_DEV *pScsiPhysDev     /* ptr to a SCSI physical device [] */
    )
     
    {
    UCHAR * secBuf;             /* a secBuf for the sector data */
    LOCAL ULONG extPartOffset;  /* for first ext part offset    */
    LOCAL ULONG currentOffset;  /* for current ext part offset  */ 
    LOCAL ULONG tmpVal;         /* tmp used when byte swapping  */
    LOCAL int extPartLevel = 0; /* static for extended parts    */
    LOCAL size_t numNames;      /* partition name array entries */
    UINT8 dosSec;               /* for temp use. Sector         */
    UINT8 dosCyl;               /* for temp use. Cylinder       */
    int i;                      /* used for loop through parts  */
    int ix;                     /* used for loop through parts  */
    int ixx;                    /* used for loop for space print*/
    int partOffset;             /* offset from partition tbl ent*/

    struct partType             /* Some partition type values & names. */
        {                       /* Only MSDOS are used in this code.   */
        UINT8 partTypeNum;      /* dosPartParseScsi() may be modified  */
        char *partTypeName;     /* to support any others if needed     */
        } partNames[] = 
            {
            {0x00, "Empty (NULL) Partition"},
            {0x01, "MSDOS Partition 12-bit FAT"},   
            {0x02, "XENIX / (slash) Partition"},        
            {0x03, "XENIX /usr Partition"}, 
            {0x04, "MSDOS 16-bit FAT <32M Partition"},  
            {0x05, "MSDOS Extended Partition"}, 
            {0x06, "MSDOS 16-bit FAT >=32M Partition"},
            {0x07, "HPFS / NTFS Partition"},
            {0x08, "AIX boot or SplitDrive Partition"},
            {0x09, "AIX data or Coherent Partition"},
            {0x0a, "OS/2 Boot Manager Partition"},
            {0x0b, "Win95 FAT32 Partition"},
            {0x0c, "Win95 FAT32 (LBA) Partition"},
            {0x0e, "Win95 FAT16 (LBA) Partition"},
            {0x0f, "Win95 Extended (LBA) Partition"},
            {0x10, "OPUS Partition"},
            {0x11, "Hidden DOS FAT12 Partition"},
            {0x12, "Compaq diagnostics Partition"},
            {0x14, "Hidden DOS FAT16 Partition"},
            {0x16, "Hidden DOS FAT16 (big) Partition"},
            {0x17, "Hidden HPFS/NTFS Partition"},
            {0x18, "AST Windows swapfile Partition"},
            {0x24, "NEC DOS Partition"},
            {0x3c, "PartitionMagic recovery Partition"},
            {0x40, "Venix 80286 Partition"},
            {0x41, "Linux/MINIX (shared with DRDOS) Partition"},
            {0x42, "SFS or Linux swap part (shared with DRDOS)"},
            {0x43, "Linux native (shared with DRDOS) Partition"},
            {0x50, "DM (disk manager) Partition"},
            {0x51, "DM6 Aux1 (or Novell) Partition"},
            {0x52, "CP/M or Microport SysV/AT Partition"},
            {0x53, "DM6 Aux3 Partition"},
            {0x54, "DM6 Partition"},
            {0x55, "EZ-Drive (disk manager) Partition"},
            {0x56, "Golden Bow (disk manager) Partition"},
            {0x5c, "Priam Edisk (disk manager) Partition"},
            {0x61, "SpeedStor Partition"},
            {0x63, "GNU HURD or Mach or Sys V/386 (ISC UNIX)"},
            {0x64, "Novell Netware 286 Partition"},
            {0x65, "Novell Netware 386 Partition"},
            {0x70, "DiskSecure Multi-Boot Partition"},
            {0x75, "PC/IX Partition"},
            {0x77, "QNX4.x Partition"},
            {0x78, "QNX4.x 2nd part Partition"},
            {0x79, "QNX4.x 3rd part Partition"},
            {0x80, "MINIX until 1.4a Partition"},
            {0x81, "MINIX / old Linux Partition"},
            {0x82, "Linux swap Partition"},
            {0x83, "Linux native Partition"},
            {0x84, "OS/2 hidden C: drive Partition"},
            {0x85, "Linux extended Partition"},
            {0x86, "NTFS volume set Partition"},
            {0x87, "NTFS volume set Partition"},
            {0x93, "Amoeba Partition"},
            {0x94, "Amoeba BBT Partition"}, 
            {0xa0, "IBM Thinkpad hibernation Partition"},
            {0xa5, "BSD/386 Partition"},
            {0xa7, "NeXTSTEP 486 Partition"},
            {0xb7, "BSDI fs Partition"},
            {0xb8, "BSDI swap Partition"},
            {0xc1, "DRDOS/sec (FAT-12) Partition"},
            {0xc4, "DRDOS/sec (FAT-16, < 32M) Partition"},
            {0xc6, "DRDOS/sec (FAT-16, >= 32M) Partition"},
            {0xc7, "Syrinx Partition"},
            {0xdb, "CP/M-Concurrent CP/M-Concurrent DOS-CTOS"},
            {0xe1, "DOS access-SpeedStor 12-bit FAT ext."},
            {0xe3, "DOS R/O or SpeedStor Partition"},
            {0xe4, "SpeedStor 16-bit FAT Ext Part. < 1024 cyl."},
            {0xf1, "SpeedStor Partition"},
            {0xf2, "DOS 3.3+ secondary Partition"},
            {0xf4, "SpeedStor large partition Partition"},
            {0xfe, "SpeedStor >1024 cyl. or LANstep Partition"},
            {0xff, "Xenix Bad Block Table Partition"},
            };


    /* get number of partition type entries */

    numNames = ((sizeof (partNames)) / (sizeof (struct partType)));


    /* setup for a read to the 1st sector, aka the MBR */

    if (extPartLevel == 0)
        {
        extPartOffset = 0;
        currentOffset = 0;
        }


    /* allocate a local secBuf for the read sectors MBR/Part data */

    if  ((secBuf = malloc (BYTES_PER_SECTOR)) == NULL)
        {
        printErr ("dosPartShowScsi error. " 
                  "Error during malloc'ing sector secBuf.\n");
        return (ERROR);
        }
  


    /* read the partition sector (LBA format) into a secBuf */

    if ((dosPartRdLbaScsi (pScsiPhysDev, currentOffset, secBuf)) 
        == ERROR)
        {
        printErr ("dosPartParseScsi error. " 
                  "dosPartRdLbaScsi returned error.\n");
        return (ERROR);
        }



    /* Check the validity of partition data with 0x55aa signature */

    if ((secBuf[PART_SIG_ADRS] != PART_SIG_MSB) || 
        (secBuf[PART_SIG_ADRS+1] != PART_SIG_LSB))
        {
        printErr ("dosPartShowParseScsi error. " 
                  "Sector secBuf contains an invalid"
                  " MSDOS partition signature 0x55aa.\n");
        return (ERROR);
        }


    /* Loop through and display data for all partitions in the list */

    for (i=0 ; i<4 ; i++)
        {
	/* setup an offset relative to DOS_BOOT_PART_TBL to entry */
	partOffset = i * (sizeof (DOS_PART_TBL));

        /* Display extended or MBR */

        if (extPartLevel == 0x0)
            {
            printf ("\n");
            printf ("\n");
            printf ("+--------------------------------------+\n");
            printf ("| Master Boot Record - Partition Table |\n");
            printf ("+--------------------------------------+");
            printf ("--------------------+\n");
            }

        else 
            {
            printf ("\n");
            printf ("+-----------------------------+\n");
            printf ("| Extended Partition Table %02d |\n", extPartLevel);
            printf ("+-----------------------------+");
            printf ("-----------------------------+\n");
            }
        

        /* display partition entry and offset */

        printf ("| Partition Entry number %02d   |", i);


        switch (i)
            {
            case 3:
                printf (" Partition Entry offset 0x1ee|\n");
                break; /* Fall through */

            case 2:
                printf (" Partition Entry offset 0x1de|\n");
                break; /* Fall through */

            case 1:
                printf (" Partition Entry offset 0x1ce|\n");
                break; /* Fall through */

            case 0:
                printf (" Partition Entry offset 0x1be|\n");
                break; /* Fall through */
            }


        printf ("+-----------------------------+");
        printf ("-----------------------------+\n");
        printf ("| Status field = 0x%02x         |",
                secBuf[DOS_BOOT_PART_TBL + partOffset + 
                       BOOT_TYPE_OFFSET]);
        
        if ((secBuf[DOS_BOOT_PART_TBL + partOffset + 
             BOOT_TYPE_OFFSET]) == (PART_IS_BOOTABLE))
            {
            printf (" Primary (bootable) Partition|\n");
            }

        else
            {
            printf (" Non-bootable Partition      |\n");
            }
        printf ("+-----------------------------");
        printf ("+-----------------------------+\n");


        /* print partition type name from array */

        for (ix=0; ix < numNames; ix++)
            {
            if ((partNames[ix].partTypeNum) == 
                (secBuf[DOS_BOOT_PART_TBL + partOffset + 
                        SYSTYPE_OFFSET]))
                {
                printf ("| Type 0x%02x: ",secBuf[DOS_BOOT_PART_TBL
                                                 + partOffset
                                                 + SYSTYPE_OFFSET]);
                printf (partNames[ix].partTypeName);

                /* fill in spaces after name, 61 char width */

                for (ixx=0; 
                     ixx != (47 - (strlen (partNames[ix].partTypeName)));
                     ixx++)
                    {
                    printf (" ");
                    }
                printf ("|\n");
                break;  /*  found type, so fall out of loop. */
                }
	        }

        printf ("+-----------------------------");
        printf ("+-----------------------------+\n");


        /* Skip the rest of a NULL/empty partition */

        if (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET] 
            == 0x0)
            {
            printf ("\n");
            continue;
            }


        /* Print out the CHS information */

        dosSec = (secBuf[DOS_BOOT_PART_TBL + partOffset
                  + STARTSEC_SEC_OFFSET] & 0x3f);
        dosCyl =  ((((secBuf[DOS_BOOT_PART_TBL + partOffset
                      + STARTSEC_SEC_OFFSET]) & 0xc0) << 2)|
                   (((secBuf[DOS_BOOT_PART_TBL + partOffset + 
                      STARTSEC_CYL_OFFSET]) & 0xff) >> 8));

        printf ("| Partition start LCHS: Cylinder %04d, Head %03d,",
                 dosCyl, secBuf[DOS_BOOT_PART_TBL + partOffset + 
                 STARTSEC_HD_OFFSET]);
        printf (" Sector %02d  |\n", dosSec);

        printf ("+-----------------------------");
        printf ("------------------------------+\n");

        /*
         * Print out the ending sector CHS information for partition
         * calculate the actual DOS CHS sector and cylinder values.
         */

        dosSec = (secBuf[DOS_BOOT_PART_TBL + partOffset
                  + ENDSEC_SEC_OFFSET] & 0x3f);

        dosCyl =  ((((secBuf[DOS_BOOT_PART_TBL + partOffset 
                             + ENDSEC_SEC_OFFSET]) & 0xc0) << 2) | 
                   (((secBuf[DOS_BOOT_PART_TBL + partOffset
                             + ENDSEC_CYL_OFFSET]) & 0xff) >> 8));

        printf ("| Partition end   LCHS: Cylinder %04d, Head %03d,",
                dosCyl, secBuf[DOS_BOOT_PART_TBL + partOffset
                               + ENDSEC_HD_OFFSET]);

        printf (" Sector %02d  |\n", dosSec);
        printf ("+-----------------------------");
        printf ("------------------------------+\n");


        /* Print absolute offset */
        USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                 + NSECTORS_OFFSET]), (&tmpVal));

        if (extPartLevel == 0x0)
            {

            printf ("| Sectors offset from MBR partition 0x%08lx",tmpVal);
            printf ("              |\n");
            }
        else 
            {
            printf ("| Sectors offset from the extended partition");
            printf (" 0x%08lx     |\n", tmpVal);
            }

        printf ("+-----------------------------");
        printf ("------------------------------+\n");


        USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                 + NSECTORS_TOTAL]), (&tmpVal));

        printf ("| Number of sectors in partition 0x%08lx", tmpVal);

        printf ("                 |\n");
        printf ("+-----------------------------");
        printf ("------------------------------+\n");

        USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset 
                 + NSECTORS_OFFSET]), (&tmpVal));

        printf ("| Sectors offset from start of disk 0x%08lx",
                tmpVal+currentOffset);

        printf ("              |\n");
        printf ("+-----------------------------");
        printf ("------------------------------+\n\n");

        }


    /* Loop through all partitions in the list to find extended */

    for (i=0 ; i<4 ; i++)
        {
	/* setup an offset relative to DOS_BOOT_PART_TBL to entry */
	partOffset = i * (sizeof (DOS_PART_TBL));

        /* if found an extended partition, call ourself to parse. */

        if ((secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
             == PART_TYPE_DOSEXT) ||
            (secBuf[DOS_BOOT_PART_TBL + partOffset + SYSTYPE_OFFSET]
             == PART_TYPE_WIN95_EXT))
            {
            
            /*
             * store the offset for first ext partition or update the 
             * offset for a logical drive within the extended partition 
             * All logical drives in the disks first extended partition 
             * are referenced via other extended partitions within the  
             * first. Offsets of logical drives are relative to the     
             * first extended partitions offset.
             */

           if (extPartLevel > 0)
                {
                USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset
                         + NSECTORS_OFFSET]), (&currentOffset));

                currentOffset += extPartOffset;
                }
 
           else  /* in MBR, setup first extended partition offset */
                {
                USWABL ((ULONG *) (&secBuf[DOS_BOOT_PART_TBL + partOffset
                         + NSECTORS_OFFSET]), (&extPartOffset));

                currentOffset = extPartOffset;
                }

            /* Update extPartLevel and parse found extended partition */

            extPartLevel++;
            dosPartShowScsi (pScsiPhysDev);
            extPartLevel--;
            }
        /* next part or end looping */
        }

    /* Clean up */
    free (secBuf);  
    return (OK);
    }
#endif /* INCLUDE_DOS_PART_SHOW */
