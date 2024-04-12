/* To test the math library */

/* Copyright 1991 Wind River Systems, Inc. */

/*
modification history
--------------------
02a13aug97,ram		for MC68LC040 need to add INCLUDE_SW_FP in config.h
			and need to use VX_FP_TASK as the options parameter
			for taskSpawn() in usrConfig.c for successful run.

01a,02apr92,smb  
*/
/*
DESCRIPTION

sin(x), cos(x) and tan(x) return trigonometric functions  of
radian  arguments.  Trigonometric argument reduction is car-
ried out with respect to the infinitely precise n.
asin(x) returns the arc sine of x in the range -n/2 to n/2.
acos(x) returns the arc cosine of x in the range 0 to n.
atan(x) returns the arc tangent of x in the range -n/2 to n/2.
atan2(y,x) and hypot(x,y) (see hypot(3M)) convert  rectangu-
lar coordinates (x,y) to polar (r,0); atan2(y,x) computes 0,
the argument or phase, by computing an arc tangent of y/x in
the range -n to n.
*/

/* include files */

#include <vxWorks.h>
#include <stdio.h>
#include <float.h>
#include <math.h> 
#include "test.h"

/* globals */
static double eps;

/* function definition for function approx */
static int approx
	(
	double d1,
	double d2
	)
	{
	return ((d2 ? fabs((d2 - d1) / d2) : fabs(d1)) < eps);
	}

/***************************************************************************
* testMath2 - A test routine for the math  library
*
*RETURNS: OK, else ASSERTION ERROR
*/

int testmath2()
	{
	int result = 0;
	static double piby4 = {0.78539816339744830962};
	static double rthalf = {0.70710678118654752440};

	eps = DBL_EPSILON * 4.0;
	result += ASSERT(approx(acos(-1.0), 4.0 * piby4));
	result += ASSERT(approx(acos(-rthalf), 3.0 * piby4));
	result += ASSERT(approx(acos(0.0), 2.0 * piby4));
	result += ASSERT(approx(acos(rthalf), piby4));
	result += ASSERT(approx(acos(1.0), 0.0));

	result += ASSERT(approx(asin(-1.0), -2.0 * piby4));
	result += ASSERT(approx(asin(-rthalf), -piby4));
	result += ASSERT(approx(asin(0.0), 0.0 ));
	result += ASSERT(approx(asin(rthalf), piby4));
	result += ASSERT(approx(asin(1.0), 2.0 * piby4));

	result += ASSERT(approx(atan(-DBL_MAX), -2.0 * piby4));
	result += ASSERT(approx(atan(-1.0), -piby4));
	result += ASSERT(approx(atan(0.0), 0.0));
	result += ASSERT(approx(atan(1.0), piby4));
	result += ASSERT(approx(atan(DBL_MAX), 2.0 * piby4));

	result += ASSERT(approx(atan2(-1.0, -1.0), -3.0 * piby4));
	result += ASSERT(approx(atan2(-1.0, 0.0), -2.0 * piby4));
	result += ASSERT(approx(atan2(-1.0, 1.0), -piby4));
	result += ASSERT(approx(atan2(0.0, 1.0), 0.0 ));
	result += ASSERT(approx(atan2(1.0, 1.0), piby4));
	result += ASSERT(approx(atan2(1.0, 0.0), 2.0 * piby4));
	result += ASSERT(approx(atan2(1.0, -1.0), 3.0 * piby4));
	result += ASSERT(approx(atan2(0.0, -1.0), 4.0 * piby4) ||
			approx(atan2(0.0, -1.0), -4.0 * piby4));

	result += ASSERT(approx(cos(-3.0 * piby4), -rthalf));
	result += ASSERT(approx(cos(-2.0 * piby4), 0.0));
	result += ASSERT(approx(cos(-piby4), rthalf));
	result += ASSERT(approx(cos(0.0), 1.0));
	result += ASSERT(approx(cos(piby4), rthalf));
	result += ASSERT(approx(cos(2.0 * piby4), 0.0));
	result += ASSERT(approx(cos(3.0 * piby4), -rthalf));
	result += ASSERT(approx(cos(4.0 * piby4), -1.0));

	result += ASSERT(approx(sin(-3.0 * piby4), -rthalf));
	result += ASSERT(approx(sin(-2.0 * piby4), -1.0));
	result += ASSERT(approx(sin(-piby4), -rthalf));
	result += ASSERT(approx(sin(0.0), 0.0));
	result += ASSERT(approx(sin(piby4), rthalf));
	result += ASSERT(approx(sin(2.0 * piby4), 1.0));
	result += ASSERT(approx(sin(3.0 * piby4), rthalf));
	result += ASSERT(approx(sin(4.0 * piby4), 0.0));

	result += ASSERT(approx(tan(-3.0 * piby4), 1.0));
	result += ASSERT(approx(tan(-piby4), -1.0));
	result += ASSERT(approx(tan(0.0), 0.0));
	result += ASSERT(approx(tan(piby4), 1.0));
	result += ASSERT(approx(tan(3.0 * piby4), -1.0));

	if (result == 0)
		fdprintf(2, "SUCCESS testing math, part 2 \n");
	   else
		fdprintf(2, "Failed %d times testing math, part 2 \n", result);
	return(result);
	}
