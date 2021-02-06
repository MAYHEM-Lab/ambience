/*
Copyright (c) 2007 The NetBSD Foundation, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
/* $NetBSD: cabs.c,v 1.1 2007/08/20 16:01:30 drochner Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<cabs>>, <<cabsf>>, <<cabsl>>---complex absolute-value

INDEX
        cabs
INDEX
        cabsf
INDEX
        cabsl

SYNOPSIS
       #include <complex.h>
       double cabs(double complex <[z]>);
       float cabsf(float complex <[z]>);
       long double cabsl(long double complex <[z]>);


DESCRIPTION
        These functions compute compute the complex absolute value 
        (also called norm, modulus, or magnitude) of <[z]>. 

        <<cabsf>> is identical to <<cabs>>, except that it performs
        its calculations on <<float complex>>.

        <<cabsl>> is identical to <<cabs>>, except that it performs
        its calculations on <<long double complex>>.

RETURNS
        The cabs* functions return the complex absolute value.

PORTABILITY
        <<cabs>>, <<cabsf>> and <<cabsl>> are ISO C99

QUICKREF
        <<cabs>>, <<cabsf>> and <<cabsl>> are ISO C99

*/


#include <complex.h>
#include <math.h>

double
cabs(double complex z)
{

	return hypot( creal(z), cimag(z) );
}
