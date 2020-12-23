/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * nanf () returns a nan.
 * Added by Cygnus Support.
 */

#include "fdlibm.h"

	float nanf(const char *unused)
{
	float x;

#if __GNUC_PREREQ (3, 3)
	x = __builtin_nanf("");
#else
	SET_FLOAT_WORD(x,0x7fc00000);
#endif
	return x;
}

#ifdef _DOUBLE_IS_32BITS

	double nan(const char *arg)
{
	return (double) nanf(arg);
}

#endif /* defined(_DOUBLE_IS_32BITS) */

