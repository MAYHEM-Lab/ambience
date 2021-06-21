/* Copyright (c) 2016 Yaakov Selkowitz <yselkowi@redhat.com> */
/*
FUNCTION
        <<clog10>>, <<clog10f>>---complex base-10 logarithm

INDEX
        clog10
INDEX
        clog10f

SYNOPSIS
       #define _GNU_SOURCE
       #include <complex.h>
       double complex clog10(double complex <[z]>);
       float complex clog10f(float complex <[z]>);


DESCRIPTION
        These functions compute the complex base-10 logarithm of <[z]>.
        <<clog10>> is equivalent to <<clog>>(<[z]>)/<<log>>(10).

        <<clog10f>> is identical to <<clog10>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        The clog10 functions return the complex base-10 logarithm value.

PORTABILITY
        <<clog10>> and <<clog10f>> are GNU extensions.

*/

#include <complex.h>
#include <math.h>

long double complex
clog10l(long double complex z)
{
	long double complex w;
	long double p, rr;

	rr = cabsl(z);
	p = log10l(rr);
	rr = atan2l(cimagl(z), creall(z)) * M_IVLN10;
	w = p + rr * (long double complex) I;
	return w;
}
