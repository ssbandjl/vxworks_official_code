/* intGen.h - header for intGen-intSnyc demo programs*/

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm   added copyright.
01a,14jan92,ms   written.
*/

#define INTLEVEL   1  /* interrupt level */


#if (CPU_FAMILY == I960)
#define	INTNUM	18    /* this value is target specific (hkv960 target is used
                       * in this example) for interrupt level 1. 
                       * One may need to alter this value.
                       * Please refer to <target name>.h file in your
                       * target directory for more details.
                       */
#endif                /* CPU_FAMILY==I960 */      

#if     (CPU_FAMILY == SPARC)
#define	INTNUM	18	    /* Autovector 2 for level 1 interrupt */
#define INT_NUM_INTSYNC    (IVEC_TO_INUM (IV_AUTOVEC_2))
#endif                      /* CPU_FAMILY==SPARC */

#if     (CPU_FAMILY == MC680X0)
#define INTNUM   255        /* interrupt number*/
#endif                      /* CPU_FAMILY==MC680X0 */



