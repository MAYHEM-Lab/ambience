/* Copyright (c) 2002-2005  Michael Stumpf  <mistumpf@de.pepperl-fuchs.com>
   Copyright (c) 2006,2008  Dmitry Xmelkov

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/* $Id: strtof.c 2191 2010-11-05 13:45:57Z arcanum $ */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>		/* INFINITY, NAN		*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* PSTR() is not used to save 1 byte per string: '\0' at the tail.	*/
static const char pstr_inf[] = {'I','N','F'};
static const char pstr_inity[] = {'I','N','I','T','Y'};
static const char pstr_nan[] = {'N','A','N'};

/**  The strtof() function converts the initial portion of the string pointed
     to by \a nptr to float representation.

     The expected form of the string is an optional plus ( \c '+' ) or minus
     sign ( \c '-' ) followed by a sequence of digits optionally containing
     a decimal-point character, optionally followed by an exponent.  An
     exponent consists of an \c 'E' or \c 'e', followed by an optional plus
     or minus sign, followed by a sequence of digits.

     Leading white-space characters in the string are skipped.

     The strtof() function returns the converted value, if any.

     If \a endptr is not \c NULL, a pointer to the character after the last
     character used in the conversion is stored in the location referenced by
     \a endptr.

     If no conversion is performed, zero is returned and the value of
     \a nptr is stored in the location referenced by \a endptr.

     If the correct value would cause overflow, plus or minus \c INFINITY is
     returned (according to the sign of the value), and \c ERANGE is stored
     in \c errno.  If the correct value would cause underflow, zero is
     returned and \c ERANGE is stored in \c errno.
 */

#include "stdio_private.h"

float
strtof (const char * nptr, char ** endptr)
{
    uint32_t u32;
    float flt;
    int u32digits = 0;
    unsigned char c;
    int exp;

    unsigned char flag;
#define FL_MINUS    0x01	/* number is negative	*/
#define FL_ANY	    0x02	/* any digit was readed	*/
#define FL_OVFL	    0x04	/* overflow was		*/
#define FL_DOT	    0x08	/* decimal '.' was	*/
#define FL_MEXP	    0x10	/* exponent 'e' is neg.	*/

    if (endptr)
	*endptr = (char *)nptr;

    do {
	c = *nptr++;
    } while (isspace (c));

    flag = 0;
    if (c == '-') {
	flag = FL_MINUS;
	c = *nptr++;
    } else if (c == '+') {
	c = *nptr++;
    }

    if (!strncmp (nptr - 1, pstr_inf, 3)) {
	nptr += 2;
	if (!strncmp (nptr, pstr_inity, 5))
	    nptr += 5;
	if (endptr)
	    *endptr = (char *)nptr;
	return flag & FL_MINUS ? -INFINITY : +INFINITY;
    }

    /* NAN() construction is not realised.
       Length would be 3 characters only.	*/
    if (!strncmp (nptr - 1, pstr_nan, 3)) {
	if (endptr)
	    *endptr = (char *)nptr + 2;
	return NAN;
    }

    u32 = 0;
    exp = 0;
    while (1) {

	c -= '0';

	if (c <= 9) {
	    flag |= FL_ANY;
	    if (flag & FL_OVFL) {
		if (!(flag & FL_DOT))
		    exp += 1;
	    } else {
		if (flag & FL_DOT)
		    exp -= 1;
		/* u32 = u32 * 10 + c	*/
		u32 = (((u32 << 2) + u32) << 1) + c;
		if (u32) {
		    u32digits++;
		    if (u32digits > 8)
			flag |= FL_OVFL;
		}
	    }

	} else if (c == (('.'-'0') & 0xff)  &&  !(flag & FL_DOT)) {
	    flag |= FL_DOT;
	} else {
	    break;
	}
	c = *nptr++;
    }

    if (c == (('e'-'0') & 0xff) || c == (('E'-'0') & 0xff))
    {
	int i;
	c = *nptr++;
	i = 2;
	if (c == '-') {
	    flag |= FL_MEXP;
	    c = *nptr++;
	} else if (c == '+') {
	    c = *nptr++;
	} else {
	    i = 1;
	}
	c -= '0';
	if (c > 9) {
	    nptr -= i;
	} else {
	    i = 0;
	    do {
		if (i < 3200)
		    i = (((i << 2) + i) << 1) + c;	/* i = 10*i + c	*/
		c = *nptr++ - '0';
	    } while (c <= 9);
	    if (flag & FL_MEXP)
		i = -i;
	    exp += i;
	}
    }

    if ((flag & FL_ANY) && endptr)
	*endptr = (char *)nptr - 1;

    if (u32 == 0) {
	flt = 0;
    }
    else if (u32digits + exp <= -46 || (u32 == 0)) {
	// Number is less than 1e-46, which should be rounded down to 0; return 0.0.
	flt = 0;
    }
    else if (u32digits + exp >= 40) {
	// Number is larger than 1e+39, which should be rounded to +/-Infinity.
	flt = INFINITY;
    }
    else
	flt = __atof_engine(u32, exp);

    if (flag & FL_MINUS)
	flt = -flt;

    return flt;
}
