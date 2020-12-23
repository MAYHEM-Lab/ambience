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
#include "test.h"
 one_line_type modff_vec[] = {
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000 },   /* qnan=f(qnan, &qnan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0x7ff80000, 0x00000000 },   /* qnan=f(snan, &qnan) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x7ff00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* +0=f(+inf, &+inf) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0xfff00000, 0x00000000, 0xfff00000, 0x00000000 },   /* -0=f(-inf, &-inf) */
0,};
void test_modff(m)   {run_vector_1(m,modff_vec,(char *)(modff),"modff","ffp");   }
