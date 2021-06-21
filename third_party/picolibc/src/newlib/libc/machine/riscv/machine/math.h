/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE_MATH_H_
#define _MACHINE_MATH_H_

#ifdef __riscv_flen

#define FCLASS_NEG_INF       (1 << 0)
#define FCLASS_NEG_NORMAL    (1 << 1)
#define FCLASS_NEG_SUBNORMAL (1 << 2)
#define FCLASS_NEG_ZERO      (1 << 3)
#define FCLASS_POS_ZERO      (1 << 4)
#define FCLASS_POS_SUBNORMAL (1 << 5)
#define FCLASS_POS_NORMAL    (1 << 6)
#define FCLASS_POS_INF       (1 << 7)
#define FCLASS_SNAN          (1 << 8)
#define FCLASS_QNAN          (1 << 9)


#define FCLASS_INF           (FCLASS_NEG_INF | FCLASS_POS_INF)
#define FCLASS_ZERO          (FCLASS_NEG_ZERO | FCLASS_POS_ZERO)
#define FCLASS_NORMAL        (FCLASS_NEG_NORMAL | FCLASS_POS_NORMAL)
#define FCLASS_SUBNORMAL     (FCLASS_NEG_SUBNORMAL | FCLASS_POS_SUBNORMAL)
#define FCLASS_NAN           (FCLASS_SNAN | FCLASS_QNAN)
#endif

/**
 * Not availabe for all compilers.
 * In case of absence, fall back to normal function calls
 */
#ifdef __GNUC_GNU_INLINE__
# define __declare_riscv_macro(type) extern __inline type __attribute((gnu_inline, always_inline))
# define __declare_riscv_macro_fclass(type) extern __inline type __attribute((gnu_inline, always_inline))
#else
# define __declare_riscv_macro_fclass(type) static __inline type
#endif

#if defined (__riscv_flen) && __riscv_flen >= 64
__declare_riscv_macro_fclass(long)
_fclass_d(double x)
{
	long fclass;
	__asm __volatile ("fclass.d\t%0, %1" : "=r" (fclass) : "f" (x));
	return fclass;
}
#endif

#if defined(__riscv_flen) && __riscv_flen >= 64 && defined(__GNUC_GNU_INLINE__)

/* Double-precision functions */
__declare_riscv_macro(double)
copysign(double x, double y)
{
	double result;
	__asm__("fsgnj.d\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_riscv_macro(double)
fabs(double x)
{
	double result;
	__asm__("fabs.d\t%0, %1" : "=f"(result) : "f"(x));
	return result;
}

__declare_riscv_macro(double)
fmax (double x, double y)
{
	double result;
	__asm__("fmax.d\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_riscv_macro(double)
fmin (double x, double y)
{
	double result;
	__asm__("fmin.d\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_riscv_macro(int)
finite(double x)
{
	long fclass = _fclass_d (x);
	return (fclass & (FCLASS_INF|FCLASS_NAN)) == 0;
}

__declare_riscv_macro(int)
__fpclassifyd (double x)
{
  long fclass = _fclass_d (x);

  if (fclass & FCLASS_ZERO)
    return FP_ZERO;
  else if (fclass & FCLASS_NORMAL)
    return FP_NORMAL;
  else if (fclass & FCLASS_SUBNORMAL)
    return FP_SUBNORMAL;
  else if (fclass & FCLASS_INF)
    return FP_INFINITE;
  else
    return FP_NAN;
}

#ifndef isinf
__declare_riscv_macro(int)
isinf (double x)
{
	long fclass = _fclass_d (x);
	return (fclass & FCLASS_INF);
}
#endif

#ifndef isnan
__declare_riscv_macro(int)
isnan (double x)
{
	long fclass = _fclass_d (x);
	return (fclass & FCLASS_NAN);
}
#endif

__declare_riscv_macro(double)
__ieee754_sqrt (double x)
{
	double result;
	__asm__("fsqrt.d %0, %1" : "=f" (result) : "f" (x));
	return result;
}

#ifdef _IEEE_LIBM
__declare_riscv_macro(double)
sqrt (double x)
{
	return __ieee754_sqrt(x);
}
#endif

#if HAVE_FAST_FMA
__declare_riscv_macro(double)
fma (double x, double y, double z)
{
	double result;
	__asm__("fmadd.d %0, %1, %2, %3" : "=f" (result) : "f" (x), "f" (y), "f" (z));
	return result;
}
#endif

#endif /* defined(__riscv_flen) && __riscv_flen >= 64 && defined(__GNUC_GNU_INLINE__) */

#if defined(__riscv_flen) && __riscv_flen >= 32
__declare_riscv_macro_fclass(long)
_fclass_f(float x)
{
	long fclass;
	__asm __volatile ("fclass.s\t%0, %1" : "=r" (fclass) : "f" (x));
	return fclass;
}
#endif

#if defined(__riscv_flen) && __riscv_flen >= 32 && defined(__GNUC_GNU_INLINE__)

/* Single-precision functions */
__declare_riscv_macro(float)
copysignf(float x, float y)
{
	float result;
	__asm__("fsgnj.s\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_riscv_macro(float)
fabsf (float x)
{
	float result;
	__asm__("fabs.s\t%0, %1" : "=f"(result) : "f"(x));
	return result;
}

__declare_riscv_macro(float)
fmaxf (float x, float y)
{
	float result;
	__asm__("fmax.s\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_riscv_macro(float)
fminf (float x, float y)
{
	float result;
	__asm__("fmin.s\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_riscv_macro(int)
finitef(float x)
{
	long fclass = _fclass_f (x);
	return (fclass & (FCLASS_INF|FCLASS_NAN)) == 0;
}

__declare_riscv_macro(int)
__fpclassifyf (float x)
{
  long fclass = _fclass_f (x);

  if (fclass & FCLASS_ZERO)
    return FP_ZERO;
  else if (fclass & FCLASS_NORMAL)
    return FP_NORMAL;
  else if (fclass & FCLASS_SUBNORMAL)
    return FP_SUBNORMAL;
  else if (fclass & FCLASS_INF)
    return FP_INFINITE;
  else
    return FP_NAN;
}

__declare_riscv_macro(int)
isinff (float x)
{
	long fclass = _fclass_f (x);
	return (fclass & FCLASS_INF);
}

__declare_riscv_macro(int)
isnanf (float x)
{
	long fclass = _fclass_f (x);
	return (fclass & FCLASS_NAN);
}

__declare_riscv_macro(float)
__ieee754_sqrtf (float x)
{
	float result;
	__asm__("fsqrt.s %0, %1" : "=f" (result) : "f" (x));
	return result;
}

#ifdef _IEEE_LIBM
__declare_riscv_macro(float)
sqrtf (float x)
{
	return __ieee754_sqrtf(x);
}
#endif

#endif /* defined(__riscv_flen) && __riscv_flen >= 32 && defined(__GNUC_GNU_INLINE__) */

#if defined(HAVE_FAST_FMAF) && defined(__GNUC_GNU_INLINE__)
__declare_riscv_macro(float)
fmaf (float x, float y, float z)
{
	float result;
	__asm__("fmadd.s %0, %1, %2, %3" : "=f" (result) : "f" (x), "f" (y), "f" (z));
	return result;
}
#endif

#endif /* _MACHINE_MATH_H_ */
