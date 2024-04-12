/* dosPartLibAta.h - partition parsing/mounting/display routines. */

/*
modification history
--------------------
01b,15sep97,jkf   removed LCHS support.  LCHS only useful for MSDOS
                  versions below 3.3.
01a,01sep97,jkf   written.
*/


#ifndef __INCdosPartLibAtah    /* Avoid any multiple inclusions */
#define __INCdosPartLibAtah

#ifdef __cplusplus             /* Maintain C++ compatibility */
extern "C" {
#endif /* __cplusplus */

/* includes */

#include "lstLib.h"             /* lstLib headers (vxWorks)         */
#include "dosFsLib.h"           /* dos file system (vxWorks)        */

/* imports */

/* defines */                  

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

extern STATUS dosPartMountAta  /* Mounts partitions on drive/ctrl  */
    (
    BOOL ctrl,                 /* (0: Master/Single or 1: Slave.)  */
    BOOL drive                 /* 0:primary ctrl, 1:secondary ctrl */
    );


#ifdef INCLUDE_PART_SHOW       /* Include partition show routines  */
extern STATUS dosPartShowAta   /* Show partition data, all entries */
    (                   
    BOOL ctrl,                 /* 0:primary ctrl, 1:secondary ctrl */
    BOOL drive                 /* 0: Master/Single or 1: Slave.    */
    );
#endif /* ifdef INCLUDE_PART_SHOW */


#else /* __STDC__ */

extern STATUS dosPartMountAta(); /* Mounts partitions on drive/ctrl  */

#ifdef INCLUDE_PART_SHOW         /* Include partition show routines  */
extern STATUS dosPartShowAta();  /* Show partition data, all entries */
#endif /* ifdef INCLUDE_PART_SHOW */

#endif /* __STDC__ */


#ifdef __cplusplus      /* Maintain C++ compatibility */
}
#endif /* __cplusplus */


#endif /* __INCdosPartLibAtah */
