/* To test the locale ANSI C library */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,15aug97,whjr    added documentation
01a,02apr92,smb  
*/


/*  INCLUDES  */

#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <locale.h>

#include "test.h"

/*
DESCRIPTION
assert:
If an expression is false (that is, equal to zero), the assert() macro
writes information about the failed call to standard erro in an
implementation-defined format.  It then call abort().  The diagnostic
information includes:

     -the text of the argument
     -the name of the source file (value of preprocessor machro __FILE__)
     -the source line number (value of preprocessor machro __LINE__)
     
RETURNS N/A.

strcmp:
This routine compares string s1 to s2 lexicographically.
RETURNS an integer greater than, equal to,or less than 0, according to whether s1
is lexicographically greater than, equal to, or less than s2, respectively.

strlen:
This routine returns the number of characters in s, not including EOS.
RETURNS the number of non-null characters in the string.

A function to test Locale */

int testclocale
	(
	struct lconv *p
	)
	{
	int res = 0;

	res += ASSERT(strcmp(p->thousands_sep, "") == 0);
	res += ASSERT(strcmp(p->grouping, "") == 0);
	res += ASSERT(strcmp(p->int_curr_symbol, "") == 0);
	res += ASSERT(strcmp(p->currency_symbol, "") == 0);
	res += ASSERT(strcmp(p->mon_decimal_point, "") == 0);
	res += ASSERT(strcmp(p->mon_thousands_sep, "") == 0);
	res += ASSERT(strcmp(p->mon_grouping, "") == 0);
	res += ASSERT(strcmp(p->positive_sign, "") == 0);
	res += ASSERT(strcmp(p->negative_sign, "") == 0);
	res += ASSERT(strcmp(p->decimal_point, ".") == 0);
	res += ASSERT(p->int_frac_digits == CHAR_MAX);
	res += ASSERT(p->frac_digits == CHAR_MAX);
	res += ASSERT(p->p_cs_precedes == CHAR_MAX);
	res += ASSERT(p->p_sep_by_space == CHAR_MAX);
	res += ASSERT(p->n_cs_precedes == CHAR_MAX);
	res += ASSERT(p->n_sep_by_space == CHAR_MAX);
	res += ASSERT(p->p_sign_posn == CHAR_MAX);
	res += ASSERT(p->n_sign_posn == CHAR_MAX);
	return(res);
	}


/* A function to test Locale */

int testlocale(void)
	{
	int result = 0;
	struct lconv *p = NULL;
	char buf[32], *s;

	result += ASSERT((p = localeconv()) != NULL);
	result += testclocale(p);
	result += ASSERT((s = setlocale(LC_ALL, NULL)) != NULL);
	result += ASSERT(strlen(s) < sizeof (buf));
	strcpy(buf, s);
	result += ASSERT(setlocale(LC_ALL, "") != NULL);
	result += ASSERT(localeconv() != NULL);

	result += ASSERT((s = setlocale(LC_MONETARY, "C")) != NULL);
#if ALL_OUTPUTS
	fdprintf(2,strcmp(s, "C") ? "Native locale differs from \"C\"\n" 
			    : "Native locale same as \"C\"\n");
#endif
	result += ASSERT(setlocale(LC_NUMERIC, "C") != NULL);
	result += ASSERT((p = localeconv()) != NULL);
	result += testclocale(p);
	result += ASSERT(setlocale(LC_ALL, buf) != NULL);
	result += ASSERT((p = localeconv()) != NULL);
	result += testclocale(p);

	if (result == 0)
		fdprintf(2, "SUCCESS testing locale\n");
	   else
		fdprintf(2, "Failed %d times testing locale\n", result);
	return(result);
    	}

