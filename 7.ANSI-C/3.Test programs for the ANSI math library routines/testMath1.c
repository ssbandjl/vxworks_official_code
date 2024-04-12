/* To test the math library */

/* Copyright 1992 Wind River Systems, Inc. */

/*
modification history
--------------------
01a,05nov97,mm      fixed ceil of 0.0, expected 0.0 not 1.0 
01a,02apr92,smb     written 
*/
/*
DESCRIPTION
This program is a test routine for the math library.
Some of the functions being tested here are as follows:

double ceil(double x);
ceil(x) returns the least  integral  value  greater  than  or
equal  to  x.

double floor(double x);
floor(x) returns the greatest integral  value  less  than  or
equal  to  x.

double fabs(double x);
fabs(x) returns the absolute value of x.

double fmod(double x, double y);
fmod(x,y) returns a remainder  of  x  with respect  to  y.

double modf(double value, double *iptr);
modf() returns the signed  fractional part of value and store the integral part
indirectly in the location pointed to by iptr.

double ldexp(double value, int exp);
ldexp() returns the quantity value *  2exp.    

double frexp(double value, int *eptr);
frexp() returns  the  mantissa  of  a  double  value, and stores the
exponent indirectly in the location pointed to by eptr.      
If value is zero, both results returned by frexp() are zero.

*/

/* include files */

#include <vxWorks.h>
#include <stdio.h>
#include <math.h> 
#include <float.h> 
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
	if (d2 != 0)
		return (fabs((d2 - d1) / d2) < eps);
	else
		return (fabs(d1) < eps);
	}


/***************************************************************************
* testmath1 - A test routine for the math library.
*
* RETURNS: OK, else ASSERTION ERROR  
*/

int testmath1()
	{
	double hughVal,x;
	int xexp;
	int result = 0;


	hughVal = HUGE_VAL;
	eps = DBL_EPSILON * 4.0;
	result += ASSERT(ceil(0.0) == 0.0);

	result += ASSERT(ceil(-5.1) == -5.0);
	result += ASSERT(ceil(-5.0) == -5.0);
	result += ASSERT(ceil(-4.9) == -4.0);
	result += ASSERT(ceil(0.0) == 0.0);
	result += ASSERT(ceil(4.9) == 5.0);
	result += ASSERT(ceil(5.0) == 5.0);
	result += ASSERT(ceil(5.1) == 6.0);

	result += ASSERT(fabs(-5.0) == 5.0);
	result += ASSERT(fabs(0.0) == 0.0);
	result += ASSERT(fabs(5.0) == 5.0);
	result += ASSERT(floor(-5.1) == -6.0);
	result += ASSERT(floor(-5.0) == -5.0);
	result += ASSERT(floor(-4.9) == -5.0);
	result += ASSERT(floor(0.0) == 0.0);
	result += ASSERT(floor(4.9) == 4.0);
	result += ASSERT(floor(5.0) == 5.0);
	result += ASSERT(floor(5.1) == 5.0);

	result += ASSERT(fmod(-7.0, 3.0) == -1.0);
	result += ASSERT(fmod(-3.0, 3.0) == 0.0);
	printf("fmod(-3.0, 3.0) returns %f\n", fmod(-3.0, 3.0));
	result += ASSERT(fmod(-2.0, 3.0) == -2.0);
	result += ASSERT(fmod(0.0, 3.0) == 0.0);
	result += ASSERT(fmod(2.0, 3.0) == 2.0);
	result += ASSERT(fmod(3.0, 3.0) == 0.0);
	result += ASSERT(fmod(7.0, 3.0) == 1.0);

	result += ASSERT(approx(frexp(-3.0, &xexp), -0.75) && xexp == 2);
	result += ASSERT(approx(frexp(-0.5, &xexp), -0.5) && xexp == 0);
	result += ASSERT(frexp(0.0, &xexp) == 0.0  && xexp == 0);
	printf("frexp(0.0, &xexp) returns %f, %d\n", frexp(0.0, &xexp), xexp);
	result += ASSERT(approx(frexp(0.33, &xexp), 0.66) && xexp == -1);
	result += ASSERT(approx(frexp(0.66, &xexp), 0.66) && xexp == 0);
	result += ASSERT(approx(frexp(96.0, &xexp), 0.75) && xexp == 7);
	result += ASSERT(ldexp(-3.0, 4) == -48.0);
	result += ASSERT(ldexp(-0.5, 0) == -0.5);
	result += ASSERT(ldexp(0.0, 36) == 0.0);
	printf("ldexp(0.0, 36) returns %f\n", ldexp(0.0, 36));
	result += ASSERT(approx(ldexp(0.66, -1), 0.33));
	result += ASSERT(ldexp(96, -3) == 12.0);
	result += ASSERT(approx(modf(-11.7, &x), -11.7 + 11.0) && x== -11.0);
	result += ASSERT(modf(-0.5, &x) == -0.5 && x == -0.0);
	result += ASSERT(modf(0.0, &x) == 0.0 && x == 0.0);
	result += ASSERT(modf(0.6, &x) == 0.6 && x == 0.0);
	result += ASSERT(modf(12.0, &x) == 0.0 && x == 12.0);


	if (result == 0)
		fdprintf(2, "SUCCESS testing math, part 1 \n");
	   else
		fdprintf(2, "Failed %d times testing math, part 1 \n", result);
	return(result);
	}
