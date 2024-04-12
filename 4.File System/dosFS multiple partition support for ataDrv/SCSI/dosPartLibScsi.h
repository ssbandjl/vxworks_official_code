/* dosPartLibScsi.h - partition parsing/mounting/display routines. */

/*
modification history
--------------------
01c,10mar98,dmr   fixed error in title text (only)
01b,15sep97,jkf   removed LCHS support.  LCHS only useful for MSDOS
                  versions below 3.3.
01a,01sep97,jkf   written.
*/


#ifndef __INCdosPartLibScsih    /* Avoid any multiple inclusions */
#define __INCdosPartLibScsih

#ifdef __cplusplus             /* Maintain C++ compatibility */
extern "C" {
#endif /* __cplusplus */

/* includes */

#include "lstLib.h"             /* lstLib headers (vxWorks)         */
#include "dosFsLib.h"           /* dos file system (vxWorks)        */
#define INCLUDE_SCSI2           /* Do not move Place above scsiLib.h*/
#include "scsiLib.h"            /* SCSI	support, below INCLUDE_SCSI2*/

/* imports */

/* defines */                  

/* 
 * There is one user definable macro for dosPartLibScsi:
 * 
 * 1.) INCLUDE_PART_SHOW.  Undefining this will remove
 *     the dosPartShowAta routine.  This is to reduce
 *     code size when the show routine is not needed.
 * 
 */


#define INCLUDE_PART_SHOW       /* undef to remove dosPartShowAta() */
                                /* to reduce code size              */


/* typedefs */

typedef struct                  /* struct for mountable parts   */
    {
    NODE partNode;              /* head,tail,& count ptr for dll*/
    ULONG dosPartBlkOffset;     /* block offset for xxDevCreate */
    ULONG dosPartNumBlks;       /* block size for xxDevCreate   */
    DOS_PART_TBL dosPart;       /* real dos partition tbl data  */
    } PART_NODE;


/* statics */

/* globals */

/* function declarations */

#if defined (__STDC__) || defined (__cplusplus)

extern STATUS dosPartMountScsi     /* mounts partitions on drive/ctrl  */
    (
    SCSI_CTRL * pSysScsiCtrl       /* SCSI_CTRL pointer */
    );

#ifdef INCLUDE_PART_SHOW       /* Include partition show routines  */
extern STATUS dosPartShowAta   /* Show partition data, all entries */
    (                   
    BOOL ctrl,                 /* 0:primary ctrl, 1:secondary ctrl */
    BOOL drive                 /* 0: Master/Single or 1: Slave.    */
    );
#endif /* ifdef INCLUDE_PART_SHOW */

#else /* __STDC__ */

extern STATUS dosPartMountScsi();     /* mounts partitions on drive/ctrl  */

#ifdef INCLUDE_PART_SHOW         /* Include partition show routines  */
extern STATUS dosPartShowAta();  /* Show partition data, all entries */
#endif /* ifdef INCLUDE_PART_SHOW */

#endif /* __STDC__ */


#ifdef __cplusplus      /* Maintain C++ compatibility */
}
#endif /* __cplusplus */


#endif /* __INCdosPartLibAtah */
