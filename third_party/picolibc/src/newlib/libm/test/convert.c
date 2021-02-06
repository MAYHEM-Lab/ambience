/*
 * Copyright (c) 1994 Cygnus Support.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* Test conversions */

#define IN_CONVERT
#include "test.h"
//#include <_ansi.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static char buffer[500];

extern double_type doubles[];

//#define GENERATE_VECTORS

/* TEST ATOF  ATOFF */

double_type *pd = doubles;

#ifdef _IO_FLOAT_EXACT
#define CONVERT_BITS_DOUBLE	64
#define CONVERT_BITS_FLOAT	32
#else
#define CONVERT_BITS_DOUBLE	61
#define CONVERT_BITS_FLOAT	30
#endif

void
test_strtod (void)
{
  char *tail;
  double v;
  /* On average we'll loose 1/2 a bit, so the test is for within 1 bit  */
  v = strtod(pd->string, &tail);
  test_mok(v, pd->value, CONVERT_BITS_DOUBLE);
  test_iok(tail - pd->string, pd->endscan);
}

void
test_strtof (void)
{
  char *tail;
  float v;
  /* On average we'll loose 1/2 a bit, so the test is for within 1 bit  */
  v = strtof(pd->string, &tail);
  test_mok((double) v, pd->value, CONVERT_BITS_FLOAT);
  test_iok(tail - pd->string, pd->endscan);
}

#if defined(_HAVE_LONG_DOUBLE) && (__LDBL_MANT_DIG__ == 64 || defined(TINY_STDIO))
#define HAVE_STRTOLD
#endif

#ifdef HAVE_STRTOLD
void
test_strtold (void)
{
  char *tail;
  long double v;
  /* On average we'll loose 1/2 a bit, so the test is for within 1 bit  */
  v = strtold(pd->string, &tail);
  test_mok(v, pd->value, CONVERT_BITS_DOUBLE);
  test_iok(tail - pd->string, pd->endscan);
}
#endif

void
test_atof (void)
{
  test_mok(atof(pd->string), pd->value, CONVERT_BITS_DOUBLE);
}

#ifndef NO_NEWLIB
void
test_atoff (void)
{
  float v = atoff(pd->string);
  test_mok((double) v, pd->value, 30);
}
#endif


static
void 
iterate (void (*func) (void),
       char *name)
{

  newfunc(name);
  pd = doubles;
  while (pd->string) {
    line(pd->line);
    func();
    pd++;
  }
}


extern int_type ints[];

int_type *p = ints;


static void
int_iterate (void (*func)(),
       char *name)
{
  newfunc(name);

  p = ints;
  while (p->string) {
    line(p->line);
    errno = 0;
    func();
    p++;
  }
}

void
test_strtol_base (int base,
       int_scan_type *pi,
       char *string)
{
  long r;
  char *ptr;
  errno = 0;
  r = strtol(string, &ptr, base);
  test_iok(r, pi->value);
  test_eok(errno, pi->errno_val);
  test_iok(ptr - string, pi->end);
}

void
test_strtol (void)
{
  test_strtol_base(8,&(p->octal), p->string);
  test_strtol_base(10,&(p->decimal), p->string);
  test_strtol_base(16, &(p->hex), p->string);
  test_strtol_base(0, &(p->normal), p->string);
  test_strtol_base(26, &(p->alphabetical), p->string);
}

void
test_atoi (void)
{
  if (p->decimal.errno_val == 0)
    test_iok(atoi(p->string), p->decimal.value);
}

void
test_atol (void)
{
  test_iok(atol(p->string), p->decimal.value);
  test_eok(errno, p->decimal.errno_val);
}

extern ddouble_type ddoubles[];
ddouble_type *pdd;

static inline char *
check_null(char *s) {
  if (s == NULL)
    return "(out of memory)";
  return s;
}

#ifndef NO_NEWLIB
/* test ECVT and friends */
void
test_ecvtbuf (void)
{
  int a2,a3;
  char *s;
  s =  check_null(ecvtbuf(pdd->value, pdd->e1, &a2, &a3, buffer));

  test_sok(s,pdd->estring);
  test_iok(pdd->e2,a2);
  test_iok(pdd->e3,a3);
}
#endif

void
test_ecvt (void)
{
  int a2,a3;
  char *s;
  s =  check_null(ecvt(pdd->value, pdd->e1, &a2, &a3));

  test_sok(s,pdd->estring);
  test_iok(pdd->e2,a2);
  test_iok(pdd->e3,a3);

#ifndef NO_NEWLIB
  s =  check_null(ecvtf(pdd->value, pdd->e1, &a2, &a3));

  test_scok(s,pdd->estring, 6);
  test_iok(pdd->e2,a2);
  test_iok(pdd->e3,a3);
#endif
}

#ifndef NO_NEWLIB
void
test_fcvtbuf (void)
{
  int a2,a3;
  char *s;
  s =  check_null(fcvtbuf(pdd->value, pdd->f1, &a2, &a3, buffer));

  test_scok(s,pdd->fstring,10);
  test_iok(pdd->f2,a2);
  test_iok(pdd->f3,a3);
}
#endif

void
test_gcvt (void)
{
  char *s = check_null(gcvt(pdd->value, pdd->g1, buffer));
  test_scok(s, pdd->gstring, 9);
  
#ifndef NO_NEWLIB
  s = check_null(gcvtf(pdd->value, pdd->g1, buffer)); 
  test_scok2(s, pdd->gstring, pdd->gfstring, 6);
#endif
}

void
test_fcvt (void)
{
  int a2,a3;
  char *sd;
  sd =  check_null(fcvt(pdd->value, pdd->f1, &a2, &a3));

  test_scok(sd,pdd->fstring,10);
  test_iok(pdd->f2,a2);
  test_iok(pdd->f3,a3);

#ifndef NO_NEWLIB
  char *sf;
  double v1;
  double v2;
  char *sde, *sfe;
  /* Test the float version by converting and inspecting the numbers 3
   after reconverting */
  sf =  check_null(fcvtf(pdd->value, pdd->f1, &a2, &a3));
  v1 = strtod(sd, &sde);
  v2 = strtod(sf, &sfe);
  /* float version may return fewer digits; expand to match */
  int x = strlen(sd) - strlen(sf);
  while (x-- > 0)
    v2 *= 10;
  if (strlen(sd) == 0) {
    test_iok(0, sde - sd);
    v1 = 0.0;
  }
  if (strlen(sf) == 0) {
    test_iok(0, sfe - sf);
    v2 = 0.0;
  }
  test_mok(v1, v2,30);
  test_iok(pdd->f2,a2);
  test_iok(pdd->f3,a3);
#endif
}

static void

diterate (void (*func)(),
       char *name)
{
  newfunc(name);

  pdd = ddoubles;
  while (pdd->estring) {
    line(pdd->line);
    errno = 0;
    func();
    pdd++;
  }
}


void
deltest (void)
{
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
  newfunc("rounding");
  line(1);
  sprintf(buffer,"%.2f", 9.999);
  test_sok(buffer,"10.00");
  line(2);
  sprintf(buffer,"%.2g", 1.0);
  test_sok(buffer,"1");
  line(3);
  sprintf(buffer,"%.2g", 1.2e-6);
  test_sok(buffer,"1.2e-06");
  line(4);
  sprintf(buffer,"%.0g", 1.0);
  test_sok(buffer,"1");
  line(5);
  sprintf(buffer,"%.0e",1e1);
  test_sok(buffer,"1e+01");
  line(6);  
  sprintf(buffer, "%f", 12.3456789);
  test_sok(buffer, "12.345679");
  line(7);  
  sprintf(buffer, "%6.3f", 12.3456789);
  test_sok(buffer, "12.346");
  line(8);  
  sprintf(buffer,"%.0f", 12.3456789);
  test_sok(buffer,"12");
#endif
}

/* Most of what sprint does is tested with the tests of
   fcvt/ecvt/gcvt, but here are some more */
void
test_sprint (void)
{
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
  extern sprint_double_type sprint_doubles[];
  sprint_double_type *s = sprint_doubles;
#endif
  extern sprint_int_type sprint_ints[];
  sprint_int_type *si = sprint_ints;


  newfunc( "sprintf");  


#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
  while (s->line) 
  {
    line( s->line);
    sprintf(buffer, s->format_string, s->value);
#ifdef GENERATE_VECTORS
    if (s->mag)
      printf("{__LINE__, %.15e,\t\"%s\", \"%s\", %d },\n",
	     s->value, buffer, s->format_string, s->mag);
    else
      printf("{__LINE__, %.15e,\t\"%s\", \"%s\" },\n",
	     s->value, buffer, s->format_string);
#else
    test_scok(buffer, s->result, 12); /* Only check the first 12 digs,
					 other stuff is random */
#endif
    s++;
  }
#endif

  while (si->line) 
  {
    line( si->line);
    if (strchr(si->format_string, 'l'))
      sprintf(buffer, si->format_string, (long) si->value);
    else
      sprintf(buffer, si->format_string, si->value);
#ifdef GENERATE_VECTORS
    if (si->value < 0)
      printf("__LINE__, -%#09x,\t\"%s\", \"%s\",\n",
	     -si->value, buffer, si->format_string);
    else
      printf("__LINE__, %#010x,\t\"%s\", \"%s\",\n",
	     si->value, buffer, si->format_string);
#else
    test_sok(buffer, si->result);
#endif
    si++;
  }  
}

/* Scanf calls strtod etc tested elsewhere, but also has some pattern matching skills */
void
test_scan (void)
{
  int i,j;
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
  extern sprint_double_type sprint_doubles[];
  sprint_double_type *s = sprint_doubles;
#endif
  extern sprint_int_type sprint_ints[];
  sprint_int_type *si = sprint_ints;

  newfunc( "scanf");  
  
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
  /* Test scanf by converting all the numbers in the sprint vectors
     to and from their source and making sure nothing breaks */

  while (s->line) 
  {

    double d0,d1;
    line( s->line);
    sscanf(s->result, "%lg", &d0);
    sprintf(buffer, "%.16e", d0);
    sscanf(buffer, "%lg", &d1);
    if  (s->mag)
      test_mok(d0, d1, s->mag);
    else
      test_mok(d0,d1, CONVERT_BITS_DOUBLE);
    s++;
  }
#endif

  /* And integers too */
  while (si->line) 
  {

    long d0,d1;
    
    line(si->line);
    sscanf(si->result, "%ld", &d0);
    sprintf(buffer, "%ld", d0);
    sscanf(buffer, "%ld", &d1);
    test_iok(d0,d1);
    si++;
  }

  /* And the string matching */

  sscanf("    9","%d", &i);
  test_iok(i, 9);
  sscanf("foo bar 123 zap 456","foo bar %d zap %d", &i, &j);
  test_iok(i, 123);
  test_iok(j, 456);
  
  sscanf("magicXYZZYfoobar","magic%[XYZ]", buffer);
  test_sok("XYZZY", buffer);
  sscanf("magicXYZZYfoobar","%[^XYZ]", buffer);
  test_sok("magic", buffer);
}

#ifdef GENERATE_VECTORS
static void
gen_dvec(void)
{
  char	ebuf[128];
  char	fbuf[128];
  char	gbuf[128];
  int	e_decpt, e_sign;
  int	f_decpt, f_sign;

  strcpy(ebuf, check_null(ecvt(pdd->value, pdd->e1, &e_decpt, &e_sign)));
  strcpy(fbuf, check_null(fcvt(pdd->value, pdd->f1, &f_decpt, &f_sign)));
  check_null(gcvt(pdd->value, pdd->g1, gbuf));
  printf("__LINE__, %.15e,\"%s\",%d,%d,%d,\"%s\",%d,%d,%d,\"%s\",%d,\n\n",
	 pdd->value,
	 ebuf, pdd->e1, e_decpt, e_sign,
	 fbuf, pdd->f1, f_decpt, f_sign,
	 gbuf, pdd->g1);
}
#endif

extern int _malloc_test_fail;

void
test_cvt (void)
{
  deltest();

#ifdef GENERATE_VECTORS
  diterate(gen_dvec, "gen");
#else
#ifndef NO_NEWLIB
  diterate(test_fcvtbuf,"fcvtbuf");
#endif
  diterate(test_fcvt,"fcvt/fcvtf");

  diterate(test_gcvt,"gcvt/gcvtf");
#ifndef NO_NEWLIB
  diterate(test_ecvtbuf,"ecvtbuf");
#endif
  diterate(test_ecvt,"ecvt/ecvtf");
#endif

  iterate(test_strtod, "strtod");
#ifdef HAVE_STRTOLD
  iterate(test_strtold, "strtold");
#endif

  test_scan();
  test_sprint();  
  iterate(test_atof, "atof");
#ifndef NO_NEWLIB
  iterate(test_atoff, "atoff");
#endif

  iterate(test_strtof, "strtof");

  int_iterate(test_atoi,"atoi");
  if (sizeof(int) == sizeof(long)) {
    int_iterate(test_atol,"atol");
    int_iterate(test_strtol, "strtol");
  }
}
