/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#include <string.h>
#include <picotls.h>
#include <stdint.h>
#include <stdlib.h>

extern char __data_source[];
extern char __data_start[];
extern char __data_size[];
extern char __bss_start[];
extern char __bss_size[];
extern char __tls_base[];

/* These two functions must be defined in the architecture-specific
 * code
 */

void
_start(void);

/* This is the application entry point */
int
main(int, char **);

#ifdef HAVE_INITFINI_ARRAY
extern void __libc_init_array(void);
#endif

/* After the architecture-specific chip initialization is done, this
 * function initializes the data and bss segments. Note that a static
 * block of TLS data is carefully interleaved with the regular data
 * and bss segments in picolibc.ld so that this one operation
 * initializes both. Then it runs the application code, starting with
 * any initialization functions, followed by the main application
 * entry point and finally any cleanup functions
 */

#include <picotls.h>
#include <stdio.h>

#ifndef CONSTRUCTORS
#define CONSTRUCTORS 1
#endif

static inline void
__start(void)
{
	memcpy(__data_start, __data_source, (uintptr_t) __data_size);
	memset(__bss_start, '\0', (uintptr_t) __bss_size);
#ifdef PICOLIBC_TLS
	_set_tls(__tls_base);
#endif
#if defined(HAVE_INITFINI_ARRAY) && CONSTRUCTORS
	__libc_init_array();
#endif
	int ret = main(0, NULL);
#ifdef CRT0_EXIT
	exit(ret);
#else
	(void) ret;
	for(;;);
#endif
}
