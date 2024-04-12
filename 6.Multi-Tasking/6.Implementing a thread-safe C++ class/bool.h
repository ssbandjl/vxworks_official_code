/* bool.h - Boolean definitions */

/* Copyright 1998-2010 Wind River Systems, Inc. */

/*
 * modification history
 * --------------------
 * 01a,25mar98,pai  written 
 *
 */



#ifndef		__INCBoolh
#define		__INCBoolh



#ifdef __cplusplus
    enum Bool {False, True};
#else
    typedef enum bool  { False, True } Bool;
#endif  /* __cplusplus */



#endif	/* __INCBoolh   */

