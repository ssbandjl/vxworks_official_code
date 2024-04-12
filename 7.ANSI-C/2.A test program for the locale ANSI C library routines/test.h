/* test.h - test library header file */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01a,29jul91,smb  written.
*/

#ifndef __lib_test_h
#define __lib_test_h

#include "stdio.h"

#define ASSERT(x) ((x)? 0: (fdprintf(2,"Assert: "__FILE__" %s\n", #x), 1))
#define ALL_OUTPUTS 0


#endif /* !__lib_test_h */
