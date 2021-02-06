/* Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2008  Dmitry Xmelkov
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
  POSSIBILITY OF SUCH DAMAGE.
*/

/* $Id: vfscanf.c 2191 2010-11-05 13:45:57Z arcanum $ */

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdio_private.h"

#if	!defined (SCANF_LEVEL)
#define SCANF_LEVEL SCANF_FLT
#endif

#if	SCANF_LEVEL == SCANF_MIN
# define SCANF_BRACKET	0
# define SCANF_FLOAT	0
#elif	SCANF_LEVEL == SCANF_STD
# define SCANF_BRACKET	1
# define SCANF_FLOAT	0
int vfscanf (FILE * stream, const char *fmt, va_list ap) __attribute__((weak));
#elif	SCANF_LEVEL == SCANF_FLT
# define SCANF_BRACKET	1
# define SCANF_FLOAT	1
#else
# error	 "Not a known scanf level."
#endif

typedef unsigned int width_t;

#define FL_STAR	    0x01	/* '*': skip assignment		*/
#define FL_WIDTH    0x02	/* width is present		*/
#define FL_LONG	    0x04	/* 'long' type modifier		*/
#define FL_CHAR	    0x08	/* 'char' type modifier		*/
#define FL_SHORT    0x10	/* 'short' type modifier	*/
#define FL_OCT	    0x20	/* octal number			*/
#define FL_DEC	    0x40	/* decimal number		*/
#define FL_HEX	    0x80	/* hexidecimal number		*/
#define FL_MINUS    0x100	/* minus flag (field or value)	*/

#ifndef	__AVR_HAVE_LPMX__
# if  defined(__AVR_ENHANCED__) && __AVR_ENHANCED__
#  define __AVR_HAVE_LPMX__	1
# endif
#endif

#ifndef	__AVR_HAVE_MOVW__
# if  defined(__AVR_ENHANCED__) && __AVR_ENHANCED__
#  define __AVR_HAVE_MOVW__	1
# endif
#endif

static int
scanf_getc(FILE *stream, int *lenp)
{
	int c = getc(stream);
	if (c >= 0)
		++(*lenp);
	return c;
}

static int
scanf_ungetc(int c, FILE *stream, int *lenp)
{
	c = ungetc(c, stream);
	if (c >= 0)
		--(*lenp);
	return c;
}

static void
putval (void *addr, long val, uint16_t flags)
{
    if (!(flags & FL_STAR)) {
	if (flags & FL_CHAR)
	    *(char *)addr = val;
	else if (flags & FL_LONG)
	    *(long *)addr = val;
	else if (flags & FL_SHORT)
	    *(short *)addr = val;
	else
	    *(int *)addr = val;
    }
}

static unsigned long
mulacc (unsigned long val, uint16_t flags, unsigned char c)
{
    unsigned char cnt;

    if (flags & FL_OCT) {
	cnt = 3;
    } else if (flags & FL_HEX) {
	cnt = 4;
    } else {
	val += (val << 2);
	cnt = 1;
    }

    do { val <<= 1; } while (--cnt);
    return val + c;
}

static unsigned char
conv_int (FILE *stream, int *lenp, width_t width, void *addr, uint16_t flags)
{
    unsigned long val;
    int i;

    i = scanf_getc (stream, lenp);			/* after scanf_ungetc()	*/

    switch ((unsigned char)i) {
      case '-':
        flags |= FL_MINUS;
	/* FALLTHROUGH */
      case '+':
	if (!--width || (i = scanf_getc(stream, lenp)) < 0)
	    goto err;
    }

    val = 0;
    flags &= ~FL_WIDTH;

    if (!(flags & (FL_DEC | FL_OCT)) && (unsigned char)i == '0') {
	if (!--width || (i = scanf_getc (stream, lenp)) < 0)
	    goto putval;
	flags |= FL_WIDTH;
	if ((unsigned char)(i) == 'x' || (unsigned char)(i) == 'X') {
	    flags |= FL_HEX;
	    if (!--width || (i = scanf_getc(stream, lenp)) < 0)
		goto putval;
	} else {
	    if (!(flags & FL_HEX))
		flags |= FL_OCT;
	}
    }

/* This fact is used below to parse hexidecimal digit.	*/
#if	('A' - '0') != (('a' - '0') & ~('A' ^ 'a'))
# error
#endif
    do {
	unsigned char c = i;
	c -= '0';
	if (c > 7) {
	    if (flags & FL_OCT) goto unget;
	    if (c > 9) {
		if (!(flags & FL_HEX)) goto unget;
		c &= ~('A' ^ 'a');
		c += '0' - 'A';
		if (c > 5) {
		  unget:
		    scanf_ungetc (i, stream, lenp);
		    break;
		}
		c += 10;
	    }
	}
	val = mulacc (val, flags, c);
	flags |= FL_WIDTH;
	if (!--width) goto putval;
    } while ((i = scanf_getc(stream, lenp)) >= 0);
    if (!(flags & FL_WIDTH))
	goto err;

  putval:
    if (flags & FL_MINUS) val = -val;
    putval (addr, val, flags);
    return 1;

  err:
    return 0;
}

#if  SCANF_BRACKET
static const char *
conv_brk (FILE *stream, int *lenp, width_t width, char *addr, const char *fmt)
{
    unsigned char msk[32];
    unsigned char fnegate;
    unsigned char frange;
    unsigned char cabove;
    int i;
    
    memset (msk, 0, sizeof(msk));
    fnegate = 0;
    frange = 0;
    cabove = 0;			/* init to avoid compiler warning	*/
    
    for (i = 0; ; i++) {
	unsigned char c = *fmt++;

	if (c == 0) {
	    return 0;
	} else if (c == '^' && !i) {
	    fnegate = 1;
	    continue;
	} else if (i > fnegate) {
	    if (c == ']') break;
	    if (c == '-' && !frange) {
		frange = 1;
		continue;
	    }
	}
	
	if (!frange) cabove = c;
	
	for (;;) {
	    msk[c >> 3] |= 1 << (c & 7);
	    if (c == cabove) break;
	    if (c < cabove)
		c++;
	    else
		c--;
	}

	frange = 0;
    }
    if (frange)
	msk['-'/8] |= 1 << ('-' & 7);

    if (fnegate) {
	unsigned char *p = msk;
	do {
	    unsigned char c = *p;
	    *p++ = ~c;
	} while (p != msk + sizeof(msk));
    }

    /* And now it is a flag of fault.	*/
    fnegate = 1;

    /* NUL ('\0') is consided as normal character. This is match to Glibc.
       Note, there is no method to include NUL into symbol list.	*/
    do {
	i = scanf_getc (stream, lenp);
	if (i < 0) break;
	if (!((msk[(unsigned char)i >> 3] >> (i & 7)) & 1)) {
	    scanf_ungetc (i, stream, lenp);
	    break;
	}
	if (addr) *addr++ = i;
	fnegate = 0;
    } while (--width);
    
    if (fnegate) {
	return 0;
    } else {
	if (addr) *addr = 0;
        return fmt;
    }
}
#endif	/* SCANF_BRACKET */

#if  SCANF_FLOAT

#include "dtoa_engine.h"

static const char pstr_nfinity[] = "nfinity";
static const char pstr_an[] = "an";

static unsigned char
conv_flt (FILE *stream, int *lenp, width_t width, void *addr, uint16_t flags)
{
    UINTFLOAT uint;
    int uintdigits = 0;
    FLOAT flt;
    int i;
    const char *p;
    int exp;

    uint16_t flag;

#define FL_ANY	    0x200	/* any digit was readed	*/
#define FL_OVFL	    0x400	/* overflow was		*/
#define FL_DOT	    0x800	/* decimal '.' was	*/
#define FL_MEXP	    0x1000 	/* exponent 'e' is neg.	*/

    i = scanf_getc (stream, lenp);		/* after scanf_ungetc()	*/

    flag = 0;
    switch ((unsigned char)i) {
      case '-':
        flag = FL_MINUS;
	/* FALLTHROUGH */
      case '+':
	if (!--width || (i = scanf_getc (stream, lenp)) < 0)
	    goto err;
    }

    switch (tolower (i)) {

      case 'n':
	p = pstr_an;
	goto operate_pstr;

      case 'i':
	p = pstr_nfinity;
      operate_pstr:
        {
	    unsigned char c;
	    
	    while ((c = *p++) != 0) {
		if (!--width
		    || (i = scanf_getc (stream, lenp)) < 0
		    || ((unsigned char)tolower(i) != c
			&& (scanf_ungetc (i, stream, lenp), 1)))
		{	
		    if (p == pstr_nfinity + 3)
			break;
		    goto err;
		}
	    }
        }
	flt = (p == pstr_an + 3) ? (FLOAT) NAN : (FLOAT) INFINITY;
	break;

      default:
        exp = 0;
	uint = 0;
	do {

	    unsigned char c = i - '0';
    
	    if (c <= 9) {
		flag |= FL_ANY;
		if (flag & FL_OVFL) {
		    if (!(flag & FL_DOT))
			exp += 1;
		} else {
		    if (flag & FL_DOT)
			exp -= 1;
		    uint = uint * 10 + c;
		    if (uint) {
			uintdigits++;
#ifndef PICOLIBC_FLOAT_PRINTF_SCANF
			if (flags & FL_LONG) {
			    if (uintdigits > 16)
				flag |= FL_OVFL;
			}
			else
#endif
			{
			    if (uintdigits > 8)
				flag |= FL_OVFL;
			}
		    }
	        }

	    } else if (c == (('.'-'0') & 0xff) && !(flag & FL_DOT)) {
		flag |= FL_DOT;
	    } else {
		break;
	    }
	} while (--width && (i = scanf_getc (stream, lenp)) >= 0);
    
	if (!(flag & FL_ANY))
	    goto err;
    
	if ((unsigned char)i == 'e' || (unsigned char)i == 'E')
	{
	    int expacc;

	    if (!--width || (i = scanf_getc (stream, lenp)) < 0) goto err;
	    switch ((unsigned char)i) {
	      case '-':
		flag |= FL_MEXP;
		/* FALLTHROUGH */
	      case '+':
		if (!--width) goto err;
		i = scanf_getc (stream, lenp);		/* test EOF will below	*/
	    }

	    if (!isdigit (i)) goto err;

	    expacc = 0;
	    do {
		expacc = expacc * 10 + (i - '0');
	    } while (--width && isdigit (i = scanf_getc(stream, lenp)));
	    if (flag & FL_MEXP)
		expacc = -expacc;
	    exp += expacc;
	}

	if (width && i >= 0) scanf_ungetc (i, stream, lenp);

	if (uint == 0) {
	    flt = 0;
	    break;
	}

#ifndef PICOLIBC_FLOAT_PRINTF_SCANF
	if (flags & FL_LONG)
	{
		if ((uintdigits + exp <= -324) || (uint == 0)) {
			// Number is less than 1e-324, which should be rounded down to 0; return +/-0.0.
			flt = 0.0;
			break;
		}
		if (uintdigits + exp >= 310) {
			// Number is larger than 1e+309, which should be rounded to +/-Infinity.
			flt = (FLOAT) INFINITY;
			break;
		}
		flt = __atod_engine(uint, exp);
	}
	else
#endif
	{
		if ((uintdigits + exp <= -46) || (uint == 0)) {
			// Number is less than 1e-46, which should be rounded down to 0; return 0.0.
			flt = (FLOAT) 0.0f;
			break;
		}
		if (uintdigits + exp >= 40) {
			// Number is larger than 1e+39, which should be rounded to +/-Infinity.
			flt = (FLOAT) INFINITY;
			break;
		}
		flt = (FLOAT) __atof_engine(uint, exp);
	}
	break;
    } /* switch */

    if (flag & FL_MINUS)
	flt = -flt;
    if (addr) {
#ifndef PICOLIBC_FLOAT_PRINTF_SCANF
	if (flags & FL_LONG)
	    *((double *) addr) = flt;
	else
#endif
	    *((float *) addr) = flt;
    }
    return 1;

  err:
    return 0;
}
#endif	/* SCANF_FLOAT	*/

static int skip_spaces (FILE *stream, int *lenp)
{
    int i;
    do {
	if ((i = scanf_getc (stream, lenp)) < 0)
	    return i;
    } while (isspace (i));
    scanf_ungetc (i, stream, lenp);
    return i;
}

/**
   Formatted input.  This function is the heart of the \b scanf family of
   functions.

   Characters are read from \a stream and processed in a way described by
   \a fmt.  Conversion results will be assigned to the parameters passed
   via \a ap.

   The format string \a fmt is scanned for conversion specifications.
   Anything that doesn't comprise a conversion specification is taken as
   text that is matched literally against the input.  White space in the
   format string will match any white space in the data (including none),
   all other characters match only itself. Processing is aborted as soon
   as the data and format string no longer match, or there is an error or
   end-of-file condition on \a stream.

   Most conversions skip leading white space before starting the actual
   conversion.

   Conversions are introduced with the character \b %.  Possible options
   can follow the \b %:

   - a \c * indicating that the conversion should be performed but
     the conversion result is to be discarded; no parameters will
     be processed from \c ap,
   - the character \c h indicating that the argument is a pointer
     to <tt>short int</tt> (rather than <tt>int</tt>),
   - the 2 characters \c hh indicating that the argument is a pointer
     to <tt>char</tt> (rather than <tt>int</tt>).
   - the character \c l indicating that the argument is a pointer
     to <tt>long int</tt> (rather than <tt>int</tt>, for integer
     type conversions), or a pointer to \c double (for floating
     point conversions),

   In addition, a maximal field width may be specified as a nonzero
   positive decimal integer, which will restrict the conversion to at
   most this many characters from the input stream.  This field width is
   limited to at most 255 characters which is also the default value
   (except for the <tt>%c</tt> conversion that defaults to 1).

   The following conversion flags are supported:

   - \c % Matches a literal \c % character.  This is not a conversion.
   - \c d Matches an optionally signed decimal integer; the next
     pointer must be a pointer to \c int.
   - \c i Matches an optionally signed integer; the next pointer must
     be a pointer to \c int.  The integer is read in base 16 if it
     begins with \b 0x or \b 0X, in base 8 if it begins with \b 0, and
     in base 10 otherwise.  Only characters that correspond to the
     base are used.
   - \c o Matches an octal integer; the next pointer must be a pointer to
     <tt>unsigned int</tt>.
   - \c u Matches an optionally signed decimal integer; the next
     pointer must be a pointer to <tt>unsigned int</tt>.
   - \c x Matches an optionally signed hexadecimal integer; the next
     pointer must be a pointer to <tt>unsigned int</tt>.
   - \c f Matches an optionally signed floating-point number; the next
     pointer must be a pointer to \c float.
   - <tt>e, g, F, E, G</tt> Equivalent to \c f.
   - \c s
     Matches a sequence of non-white-space characters; the next pointer
     must be a pointer to \c char, and the array must be large enough to
     accept all the sequence and the terminating \c NUL character.  The
     input string stops at white space or at the maximum field width,
     whichever occurs first.
   - \c c
     Matches a sequence of width count characters (default 1); the next
     pointer must be a pointer to \c char, and there must be enough room
     for all the characters (no terminating \c NUL is added).  The usual
     skip of leading white space is suppressed.  To skip white space
     first, use an explicit space in the format.
   - \c [
     Matches a nonempty sequence of characters from the specified set
     of accepted characters; the next pointer must be a pointer to \c
     char, and there must be enough room for all the characters in the
     string, plus a terminating \c NUL character.  The usual skip of
     leading white space is suppressed.  The string is to be made up
     of characters in (or not in) a particular set; the set is defined
     by the characters between the open bracket \c [ character and a
     close bracket \c ] character.  The set excludes those characters
     if the first character after the open bracket is a circumflex
     \c ^.  To include a close bracket in the set, make it the first
     character after the open bracket or the circumflex; any other
     position will end the set.  The hyphen character \c - is also
     special; when placed between two other characters, it adds all
     intervening characters to the set.  To include a hyphen, make it
     the last character before the final close bracket.  For instance,
     <tt>[^]0-9-]</tt> means the set of <em>everything except close
     bracket, zero through nine, and hyphen</em>.  The string ends
     with the appearance of a character not in the (or, with a
     circumflex, in) set or when the field width runs out.  Note that
     usage of this conversion enlarges the stack expense.
   - \c p
     Matches a pointer value (as printed by <tt>%p</tt> in printf()); the
     next pointer must be a pointer to \c void.
   - \c n
     Nothing is expected; instead, the number of characters consumed
     thus far from the input is stored through the next pointer, which
     must be a pointer to \c int.  This is not a conversion, although it
     can be suppressed with the \c * flag.

     These functions return the number of input items assigned, which can
     be fewer than provided for, or even zero, in the event of a matching
     failure.  Zero indicates that, while there was input available, no
     conversions were assigned; typically this is due to an invalid input
     character, such as an alphabetic character for a <tt>%d</tt>
     conversion.  The value \c EOF is returned if an input failure occurs
     before any conversion such as an end-of-file occurs.  If an error or
     end-of-file occurs after conversion has begun, the number of
     conversions which were successfully completed is returned.

     By default, all the conversions described above are available except
     the floating-point conversions and the width is limited to 255
     characters.  The float-point conversion will be available in the
     extended version provided by the library \c libscanf_flt.a.  Also in
     this case the width is not limited (exactly, it is limited to 65535
     characters).  To link a program against the extended version, use the
     following compiler flags in the link stage:

     \code
     -Wl,-u,vfscanf -lscanf_flt -lm
     \endcode

     A third version is available for environments that are tight on
     space.  In addition to the restrictions of the standard one, this
     version implements no <tt>%[</tt> specification.  This version is
     provided in the library \c libscanf_min.a, and can be requested using
     the following options in the link stage:

     \code
     -Wl,-u,vfscanf -lscanf_min -lm
     \endcode
*/
int vfscanf (FILE * stream, const char *fmt, va_list ap)
{
    unsigned char nconvs;
    unsigned char c;
    width_t width;
    void *addr;
    uint16_t flags;
    int i;
    int scanf_len = 0;
#define lenp (&scanf_len)

    nconvs = 0;

    /* Initialization of stream_flags at each pass simplifies the register
       allocation with GCC 3.3 - 4.2.  Only the GCC 4.3 is good to move it
       to the begin.	*/
    while ((c = *fmt++) != 0) {

	if (isspace (c)) {
	    skip_spaces (stream, lenp);

	} else if (c != '%'
		   || (c = *fmt++) == '%')
	{
	    /* Ordinary character.	*/
	    if ((i = scanf_getc (stream, lenp)) < 0)
		goto eof;
	    if ((unsigned char)i != c) {
		scanf_ungetc (i, stream, lenp);
		break;
	    }
	
	} else {
	    flags = 0;

	    if (c == '*') {
		flags = FL_STAR;
		c = *fmt++;
	    }

	    width = 0;
	    while ((c -= '0') < 10) {
		flags |= FL_WIDTH;
		width = width * 10 + c;
		c = *fmt++;
	    }
	    c += '0';
	    if (flags & FL_WIDTH) {
		/* C99 says that width must be greater than zero.
		   To simplify program do treat 0 as error in format.	*/
		if (!width) break;
	    } else {
		width = ~0;
	    }

	    switch (c) {
	      case 'h':
		if ((c = *fmt++) != 'h') {
#ifdef _WANT_IO_C99_FORMATS
		is_short:
#endif
		    flags |= FL_SHORT;
		    break;
		}
		flags |= FL_CHAR;
		c = *fmt++;
		break;
	      case 'l':
#ifdef _WANT_IO_C99_FORMATS
	    is_long:
#endif
		flags |= FL_LONG;
		c = *fmt++;
		break;
#ifdef _WANT_IO_C99_FORMATS
#define CHECK_INT_SIZE(letter, type)				\
	    case letter:					\
		if (sizeof(type) != sizeof(int)) {		\
		    if (sizeof(type) == sizeof(long))		\
			goto is_long;				\
		    if (sizeof(type) == sizeof(short))		\
			goto is_short;				\
		}						\
		c = *fmt++;					\
		break;

	    CHECK_INT_SIZE('j', intmax_t);
	    CHECK_INT_SIZE('z', size_t);
	    CHECK_INT_SIZE('t', ptrdiff_t);
#endif
	    }

#define CNV_BASE	"cdinopsuxX"
#if	SCANF_BRACKET
# define CNV_BRACKET	"["
#else
# define CNV_BRACKET	""
#endif
#if	SCANF_FLOAT
# define CNV_FLOAT	"efgEFG"
#else
# define CNV_FLOAT	""
#endif
#define CNV_LIST	CNV_BASE CNV_BRACKET CNV_FLOAT
	    if (!c || !strchr (CNV_LIST, c))
		break;

	    addr = (flags & FL_STAR) ? 0 : va_arg (ap, void *);

	    if (c == 'n') {
		putval (addr, (unsigned)(scanf_len), flags);
		continue;
	    }

	    if (c == 'c') {
		if (!(flags & FL_WIDTH)) width = 1;
		do {
		    if ((i = scanf_getc (stream, lenp)) < 0)
			goto eof;
		    if (addr) {
			*(char *)addr = i;
			addr = (char*)addr + 1;
		    }
		} while (--width);
		c = 1;			/* no matter with smart GCC	*/

#if  SCANF_BRACKET
	    } else if (c == '[') {
		fmt = conv_brk (stream, lenp, width, addr, fmt);
		c = (fmt != 0);
#endif

	    } else {

		if (skip_spaces (stream, lenp) < 0)
		    goto eof;
		
		switch (c) {

		  case 's':
		    /* Now we have 1 nospace symbol.	*/
		    do {
			if ((i = scanf_getc (stream, lenp)) < 0)
			    break;
			if (isspace (i)) {
			    scanf_ungetc (i, stream, lenp);
			    break;
			}
			if (addr) {
			    *(char *)addr = i;
			    addr = (char*)addr + 1;
			}
		    } while (--width);
		    if (addr) *(char *)addr = 0;
		    c = 1;		/* no matter with smart GCC	*/
		    break;

#if  SCANF_FLOAT
	          case 'p':
		  case 'x':
	          case 'X':
		    flags |= FL_HEX;
		    goto conv_int;

	          case 'd':
		  case 'u':
		    flags |= FL_DEC;
		    goto conv_int;

	          case 'o':
		    flags |= FL_OCT;
		    /* FALLTHROUGH */
		  case 'i':
		  conv_int:
		    c = conv_int (stream, lenp, width, addr, flags);
		    break;

	          default:		/* e,E,f,F,g,G	*/
		      c = conv_flt (stream, lenp, width, addr, flags);
#else
	          case 'd':
		  case 'u':
		    flags |= FL_DEC;
		    goto conv_int;

	          case 'o':
		    flags |= FL_OCT;
		    /* FALLTHROUGH */
		  case 'i':
		    goto conv_int;

		  default:			/* p,x,X	*/
		    flags |= FL_HEX;
		  conv_int:
		    c = conv_int (stream, lenp, width, addr, flags);
#endif
		}
	    } /* else */

	    if (!c) {
		if (stream->flags & (__SERR | __SEOF))
		    goto eof;
		break;
	    }
	    if (!(flags & FL_STAR)) nconvs += 1;
	} /* else */
    } /* while */
    return nconvs;

  eof:
    return nconvs ? nconvs : EOF;
}
