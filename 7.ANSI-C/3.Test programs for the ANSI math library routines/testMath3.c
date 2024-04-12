/* To test the math library */

/* Copyright 1991 Wind River Systems, Inc. */

/*
modification history
--------------------
02a,13aug97,ram 	for MC68LC040 need to add INCLUDE_SW_FP in config.h
			and need to use VX_FP_TASK as the options parameter
			for taskSpawn() in usrConfig.c for successful run.

01a,02apr92,smb  
*/
/*
DESCRIPTION

double sinh(double x);
double cosh(double x);
double tanh(double x);
These functions compute the designated direct
hyperbolic functions for real arguments.

*/

/* include files */

#include <vxWorks.h>
#include <stdio.h>
#include <float.h>
#include <math.h> 
#include "test.h"

/* globals */
static double eps;

/* funcion definition for function approx */

static int approx
	(
	double d1,
	double d2
	)
	{
	return ((d2 ? fabs((d2 - d1) / d2) : fabs(d1)) < eps);
	}
/***************************************************************************
* testMath3 -  A test routine for the math library
*
* RETURNS: OK, else ASSERTION ERROR.
*/

int testmath3(void)
	{
	int result = 0;
	static double e = {2.71828182845904523536};
	static double ln2 = {0.69314718055994530942};
	static double rthalf = {0.70710678118654752440};

	eps = DBL_EPSILON * 4.0;
	result += ASSERT(approx(cosh(-1.0), (e + 1.0/e) /2.0));
	result += ASSERT(approx(cosh(0.0), 1.0));
	result += ASSERT(approx(cosh(1.0), (e + 1.0/e) /2.0));

	result += ASSERT(approx(exp(-1.0), 1.0 / e));
	result += ASSERT(approx(exp(0.0), 1.0));
	result += ASSERT(approx(exp(ln2), 2.0));
	result += ASSERT(approx(exp(1.0), e));
	result += ASSERT(approx(exp(3.0), e * e * e));

	result += ASSERT(log(1.0) == 0.0);
	result += ASSERT(approx(log(e), 1.0));
	result += ASSERT(approx(log(e * e * e), 3.0));

	result += ASSERT(approx(log10(1.0), 0.0));
	result += ASSERT(approx(log10(5.0), 1.0 - log10(2.0)));
	result += ASSERT(approx(log10(1e5), 5.0));

	result += ASSERT(approx(pow(-2.5, 2.0), 6.25));
	result += ASSERT(approx(pow(-2.0, -3.0), -0.125));
	result += ASSERT(pow(0.0, 6.0) == 0.0);
	result += ASSERT(approx(pow(2.0, -0.5), rthalf));
	result += ASSERT(approx(pow(3.0, 4.0), 81.0));

	result += ASSERT(approx(sinh(-1.0), -(e - 1.0 / e) / 2.0));
	result += ASSERT(approx(sinh(0.0), 0.0));
	result += ASSERT(approx(sinh(1.0), (e - 1.0 / e) / 2.0));

	result += ASSERT(approx(sqrt(0.0), 0.0));
	result += ASSERT(approx(sqrt(0.5), rthalf));
	result += ASSERT(approx(sqrt(1.0), 1.0));
	result += ASSERT(approx(sqrt(2.0), 1.0 / rthalf));
	result += ASSERT(approx(sqrt(144.0), 12.0 ));

	result += ASSERT(approx(tanh(-1.0), -(e * e -1.0) / (e * e + 1.0)));
	result += ASSERT(approx(tanh(0.0), 0.0));
	result += ASSERT(approx(tanh(1.0), (e * e - 1.0) / (e * e + 1.0)));

	if (result == 0)
		fdprintf(2, "SUCCESS testing math, part 3 \n");
	   else
		fdprintf(2, "Failed %d times testing math, part 3 \n", result);
	return(result);
	}
