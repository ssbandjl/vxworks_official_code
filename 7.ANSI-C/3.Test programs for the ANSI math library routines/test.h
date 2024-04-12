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

#define ALL_OUTPUTS 0

static void assert (string, x)
    char *string;
    int x;
{
    fdprintf (2, string, x);
}

#define ASSERT(x) ((x)? 0: (assert("Assert: "__FILE__" %s\n", #x), 1))
/*
#define ASSERT(x) ((x)? 0: (fdprintf(2,"Assert: "__FILE__" %s\n", #x), 1))
*/

int tmcmp(struct tm *tm1, struct tm *tm2); /* @@@ ldt */

#endif /* !__lib_test_h */
