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
/* $NetBSD: cimag.c,v 1.2 2010/09/15 16:11:29 christos Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<cimag>>, <<cimagf>>, <<cimagl>>---imaginary part

INDEX
        cimag
INDEX
        cimagf
INDEX
        cimagl

SYNOPSIS
       #include <complex.h>
       double cimag(double complex <[z]>);
       float cimagf(float complex <[z]>);
       long double cimagl(long double complex <[z]>);


DESCRIPTION
        These functions compute the imaginary part of <[z]>.

        <<cimagf>> is identical to <<cimag>>, except that it performs
        its calculations on <<float complex>>.

        <<cimagl>> is identical to <<cimag>>, except that it performs
        its calculations on <<long double complex>>.

RETURNS
        The cimag* functions return the imaginary part value (as a real).

PORTABILITY
        <<cimag>>, <<cimagf>> and <<cimagl>> are ISO C99

QUICKREF
        <<cimag>>, <<cimagf>> and <<cimagl>> are ISO C99

*/


#include <complex.h>
#include "../common/fdlibm.h"

double
cimag(double complex z)
{
	double_complex w = { .z = z };

	return (IMAG_PART(w));
}
