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
#include "test.h"
 one_line_type y0_vec[] = {
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff33333, 0x33333333},	/* nan=f(-1.2)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff30a3d, 0x70a3d70a},	/* nan=f(-1.19)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff2e147, 0xae147ae1},	/* nan=f(-1.18)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff2b851, 0xeb851eb8},	/* nan=f(-1.17)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff28f5c, 0x28f5c28f},	/* nan=f(-1.16)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff26666, 0x66666666},	/* nan=f(-1.15)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff23d70, 0xa3d70a3d},	/* nan=f(-1.14)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff2147a, 0xe147ae14},	/* nan=f(-1.13)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff1eb85, 0x1eb851eb},	/* nan=f(-1.12)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff1c28f, 0x5c28f5c2},	/* nan=f(-1.11)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff19999, 0x99999999},	/* nan=f(-1.1)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff170a3, 0xd70a3d70},	/* nan=f(-1.09)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff147ae, 0x147ae147},	/* nan=f(-1.08)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff11eb8, 0x51eb851e},	/* nan=f(-1.07)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff0f5c2, 0x8f5c28f5},	/* nan=f(-1.06)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff0cccc, 0xcccccccc},	/* nan=f(-1.05)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff0a3d7, 0x0a3d70a3},	/* nan=f(-1.04)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff07ae1, 0x47ae147a},	/* nan=f(-1.03)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff051eb, 0x851eb851},	/* nan=f(-1.02)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff028f5, 0xc28f5c28},	/* nan=f(-1.01)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfefffff, 0xfffffffe},	/* nan=f(-1)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfefae14, 0x7ae147ac},	/* nan=f(-0.99)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfef5c28, 0xf5c28f5a},	/* nan=f(-0.98)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfef0a3d, 0x70a3d708},	/* nan=f(-0.97)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfeeb851, 0xeb851eb6},	/* nan=f(-0.96)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfee6666, 0x66666664},	/* nan=f(-0.95)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfee147a, 0xe147ae12},	/* nan=f(-0.94)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfedc28f, 0x5c28f5c0},	/* nan=f(-0.93)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfed70a3, 0xd70a3d6e},	/* nan=f(-0.92)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfed1eb8, 0x51eb851c},	/* nan=f(-0.91)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfeccccc, 0xccccccca},	/* nan=f(-0.9)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfec7ae1, 0x47ae1478},	/* nan=f(-0.89)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfec28f5, 0xc28f5c26},	/* nan=f(-0.88)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfebd70a, 0x3d70a3d4},	/* nan=f(-0.87)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfeb851e, 0xb851eb82},	/* nan=f(-0.86)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfeb3333, 0x33333330},	/* nan=f(-0.85)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfeae147, 0xae147ade},	/* nan=f(-0.84)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfea8f5c, 0x28f5c28c},	/* nan=f(-0.83)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfea3d70, 0xa3d70a3a},	/* nan=f(-0.82)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe9eb85, 0x1eb851e8},	/* nan=f(-0.81)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe99999, 0x99999996},	/* nan=f(-0.8)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe947ae, 0x147ae144},	/* nan=f(-0.79)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe8f5c2, 0x8f5c28f2},	/* nan=f(-0.78)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe8a3d7, 0x0a3d70a0},	/* nan=f(-0.77)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe851eb, 0x851eb84e},	/* nan=f(-0.76)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe7ffff, 0xfffffffc},	/* nan=f(-0.75)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe7ae14, 0x7ae147aa},	/* nan=f(-0.74)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe75c28, 0xf5c28f58},	/* nan=f(-0.73)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe70a3d, 0x70a3d706},	/* nan=f(-0.72)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe6b851, 0xeb851eb4},	/* nan=f(-0.71)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe66666, 0x66666662},	/* nan=f(-0.7)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe6147a, 0xe147ae10},	/* nan=f(-0.69)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe5c28f, 0x5c28f5be},	/* nan=f(-0.68)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe570a3, 0xd70a3d6c},	/* nan=f(-0.67)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe51eb8, 0x51eb851a},	/* nan=f(-0.66)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe4cccc, 0xccccccc8},	/* nan=f(-0.65)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe47ae1, 0x47ae1476},	/* nan=f(-0.64)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe428f5, 0xc28f5c24},	/* nan=f(-0.63)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe3d70a, 0x3d70a3d2},	/* nan=f(-0.62)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe3851e, 0xb851eb80},	/* nan=f(-0.61)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe33333, 0x3333332e},	/* nan=f(-0.6)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe2e147, 0xae147adc},	/* nan=f(-0.59)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe28f5c, 0x28f5c28a},	/* nan=f(-0.58)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe23d70, 0xa3d70a38},	/* nan=f(-0.57)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe1eb85, 0x1eb851e6},	/* nan=f(-0.56)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe19999, 0x99999994},	/* nan=f(-0.55)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe147ae, 0x147ae142},	/* nan=f(-0.54)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe0f5c2, 0x8f5c28f0},	/* nan=f(-0.53)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe0a3d7, 0x0a3d709e},	/* nan=f(-0.52)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfe051eb, 0x851eb84c},	/* nan=f(-0.51)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdfffff, 0xfffffff4},	/* nan=f(-0.5)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdf5c28, 0xf5c28f50},	/* nan=f(-0.49)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdeb851, 0xeb851eac},	/* nan=f(-0.48)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfde147a, 0xe147ae08},	/* nan=f(-0.47)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdd70a3, 0xd70a3d64},	/* nan=f(-0.46)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdccccc, 0xccccccc0},	/* nan=f(-0.45)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdc28f5, 0xc28f5c1c},	/* nan=f(-0.44)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdb851e, 0xb851eb78},	/* nan=f(-0.43)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfdae147, 0xae147ad4},	/* nan=f(-0.42)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfda3d70, 0xa3d70a30},	/* nan=f(-0.41)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd99999, 0x9999998c},	/* nan=f(-0.4)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd8f5c2, 0x8f5c28e8},	/* nan=f(-0.39)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd851eb, 0x851eb844},	/* nan=f(-0.38)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd7ae14, 0x7ae147a0},	/* nan=f(-0.37)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd70a3d, 0x70a3d6fc},	/* nan=f(-0.36)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd66666, 0x66666658},	/* nan=f(-0.35)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd5c28f, 0x5c28f5b4},	/* nan=f(-0.34)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd51eb8, 0x51eb8510},	/* nan=f(-0.33)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd47ae1, 0x47ae146c},	/* nan=f(-0.32)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd3d70a, 0x3d70a3c8},	/* nan=f(-0.31)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd33333, 0x33333324},	/* nan=f(-0.3)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd28f5c, 0x28f5c280},	/* nan=f(-0.29)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd1eb85, 0x1eb851dc},	/* nan=f(-0.28)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd147ae, 0x147ae138},	/* nan=f(-0.27)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfd0a3d7, 0x0a3d7094},	/* nan=f(-0.26)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfcfffff, 0xffffffe0},	/* nan=f(-0.25)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfceb851, 0xeb851e98},	/* nan=f(-0.24)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfcd70a3, 0xd70a3d50},	/* nan=f(-0.23)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfcc28f5, 0xc28f5c08},	/* nan=f(-0.22)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfcae147, 0xae147ac0},	/* nan=f(-0.21)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc99999, 0x99999978},	/* nan=f(-0.2)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc851eb, 0x851eb830},	/* nan=f(-0.19)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc70a3d, 0x70a3d6e8},	/* nan=f(-0.18)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc5c28f, 0x5c28f5a0},	/* nan=f(-0.17)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc47ae1, 0x47ae1458},	/* nan=f(-0.16)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc33333, 0x33333310},	/* nan=f(-0.15)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc1eb85, 0x1eb851c8},	/* nan=f(-0.14)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfc0a3d7, 0x0a3d7080},	/* nan=f(-0.13)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfbeb851, 0xeb851e71},	/* nan=f(-0.12)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfbc28f5, 0xc28f5be2},	/* nan=f(-0.11)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfb99999, 0x99999953},	/* nan=f(-0.1)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfb70a3d, 0x70a3d6c4},	/* nan=f(-0.09)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfb47ae1, 0x47ae1435},	/* nan=f(-0.08)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfb1eb85, 0x1eb851a6},	/* nan=f(-0.07)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfaeb851, 0xeb851e2d},	/* nan=f(-0.06)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfa99999, 0x9999990e},	/* nan=f(-0.05)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbfa47ae1, 0x47ae13ef},	/* nan=f(-0.04)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbf9eb851, 0xeb851da0},	/* nan=f(-0.03)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbf947ae1, 0x47ae1362},	/* nan=f(-0.02)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbf847ae1, 0x47ae1249},	/* nan=f(-0.01)*/
{63, 0,123,__LINE__, 0xc03613fa, 0xd0072758, 0x3cd19000, 0x00000000},	/* -22.078=f(9.74915e-16)*/
{63, 0,123,__LINE__, 0xc0080b2c, 0x5336d29d, 0x3f847ae1, 0x47ae16ad},	/* -3.00546=f(0.01)*/
{64, 0,123,__LINE__, 0xc00482fb, 0x0dc4d2d8, 0x3f947ae1, 0x47ae1594},	/* -2.56396=f(0.02)*/
{64, 0,123,__LINE__, 0xc00271a2, 0xcda61095, 0x3f9eb851, 0xeb851fd2},	/* -2.30549=f(0.03)*/
{63, 0,123,__LINE__, 0xc000f9a7, 0x00acda16, 0x3fa47ae1, 0x47ae1508},	/* -2.1219=f(0.04)*/
{63, 0,123,__LINE__, 0xbfffab42, 0x0311f75d, 0x3fa99999, 0x99999a27},	/* -1.97931=f(0.05)*/
{64, 0,123,__LINE__, 0xbffdcd51, 0x596ca896, 0x3faeb851, 0xeb851f46},	/* -1.86263=f(0.06)*/
{63, 0,123,__LINE__, 0xbffc3885, 0x010279bc, 0x3fb1eb85, 0x1eb85232},	/* -1.7638=f(0.07)*/
{63, 0,123,__LINE__, 0xbffad931, 0x2cb8fd01, 0x3fb47ae1, 0x47ae14c1},	/* -1.67803=f(0.08)*/
{63, 0,123,__LINE__, 0xbff9a2a6, 0xd440e8ef, 0x3fb70a3d, 0x70a3d750},	/* -1.60221=f(0.09)*/
{63, 0,123,__LINE__, 0xbff88c3d, 0xd3fcf171, 0x3fb99999, 0x999999df},	/* -1.53424=f(0.1)*/
{63, 0,123,__LINE__, 0xbff78fca, 0x39b900f3, 0x3fbc28f5, 0xc28f5c6e},	/* -1.4726=f(0.11)*/
{64, 0,123,__LINE__, 0xbff6a8be, 0x0dfec480, 0x3fbeb851, 0xeb851efd},	/* -1.4162=f(0.12)*/
{62, 0,123,__LINE__, 0xbff5d3a4, 0x60c3d13e, 0x3fc0a3d7, 0x0a3d70c6},	/* -1.36417=f(0.13)*/
{63, 0,123,__LINE__, 0xbff50dcd, 0xc13e44d0, 0x3fc1eb85, 0x1eb8520e},	/* -1.31587=f(0.14)*/
{64, 0,123,__LINE__, 0xbff45519, 0x9a9aa235, 0x3fc33333, 0x33333356},	/* -1.27078=f(0.15)*/
{64, 0,123,__LINE__, 0xbff3a7d1, 0x3d22b9d9, 0x3fc47ae1, 0x47ae149e},	/* -1.22847=f(0.16)*/
{63, 0,123,__LINE__, 0xbff3048e, 0x21fbf0fe, 0x3fc5c28f, 0x5c28f5e6},	/* -1.18861=f(0.17)*/
{63, 0,123,__LINE__, 0xbff26a27, 0x8d77c690, 0x3fc70a3d, 0x70a3d72e},	/* -1.15092=f(0.18)*/
{63, 0,123,__LINE__, 0xbff1d7a5, 0x2abcff8d, 0x3fc851eb, 0x851eb876},	/* -1.11515=f(0.19)*/
{64, 0,123,__LINE__, 0xbff14c35, 0x1831ea87, 0x3fc99999, 0x999999be},	/* -1.08111=f(0.2)*/
{64, 0,123,__LINE__, 0xbff0c724, 0x62650605, 0x3fcae147, 0xae147b06},	/* -1.04862=f(0.21)*/
{63, 0,123,__LINE__, 0xbff047d9, 0x3f6f6496, 0x3fcc28f5, 0xc28f5c4e},	/* -1.01754=f(0.22)*/
{62, 0,123,__LINE__, 0xbfef9b9d, 0x260f32c0, 0x3fcd70a3, 0xd70a3d96},	/* -0.987746=f(0.23)*/
{64, 0,123,__LINE__, 0xbfeeb120, 0xce71adbb, 0x3fceb851, 0xeb851ede},	/* -0.959122=f(0.24)*/
{63, 0,123,__LINE__, 0xbfedcf72, 0x3b7d21db, 0x3fd00000, 0x00000013},	/* -0.931573=f(0.25)*/
{64, 0,123,__LINE__, 0xbfecf5de, 0x5d066c27, 0x3fd0a3d7, 0x0a3d70b7},	/* -0.905013=f(0.26)*/
{64, 0,123,__LINE__, 0xbfec23c6, 0xa684d2d6, 0x3fd147ae, 0x147ae15b},	/* -0.879367=f(0.27)*/
{64, 0,123,__LINE__, 0xbfeb589e, 0x10edc97e, 0x3fd1eb85, 0x1eb851ff},	/* -0.854568=f(0.28)*/
{63, 0,123,__LINE__, 0xbfea93e6, 0xa3324d29, 0x3fd28f5c, 0x28f5c2a3},	/* -0.830554=f(0.29)*/
{63, 0,123,__LINE__, 0xbfe9d52f, 0x65f30ccb, 0x3fd33333, 0x33333347},	/* -0.807274=f(0.3)*/
{64, 0,123,__LINE__, 0xbfe91c12, 0xad4d3d82, 0x3fd3d70a, 0x3d70a3eb},	/* -0.784677=f(0.31)*/
{63, 0,123,__LINE__, 0xbfe86834, 0xa854a418, 0x3fd47ae1, 0x47ae148f},	/* -0.76272=f(0.32)*/
{64, 0,123,__LINE__, 0xbfe7b942, 0x2959afbe, 0x3fd51eb8, 0x51eb8533},	/* -0.741365=f(0.33)*/
{63, 0,123,__LINE__, 0xbfe70eef, 0x9ccc2e22, 0x3fd5c28f, 0x5c28f5d7},	/* -0.720573=f(0.34)*/
{63, 0,123,__LINE__, 0xbfe668f8, 0x269cb979, 0x3fd66666, 0x6666667b},	/* -0.700314=f(0.35)*/
{63, 0,123,__LINE__, 0xbfe5c71c, 0xdf9924d5, 0x3fd70a3d, 0x70a3d71f},	/* -0.680556=f(0.36)*/
{64, 0,123,__LINE__, 0xbfe52924, 0x2d8054ea, 0x3fd7ae14, 0x7ae147c3},	/* -0.661272=f(0.37)*/
{64, 0,123,__LINE__, 0xbfe48ed9, 0x3185c09e, 0x3fd851eb, 0x851eb867},	/* -0.642438=f(0.38)*/
{64, 0,123,__LINE__, 0xbfe3f80b, 0x49c44373, 0x3fd8f5c2, 0x8f5c290b},	/* -0.624029=f(0.39)*/
{63, 0,123,__LINE__, 0xbfe3648d, 0xa2beeda9, 0x3fd99999, 0x999999af},	/* -0.606025=f(0.4)*/
{64, 0,123,__LINE__, 0xbfe2d436, 0xd68e0fe9, 0x3fda3d70, 0xa3d70a53},	/* -0.588405=f(0.41)*/
{63, 0,123,__LINE__, 0xbfe246e0, 0x97bdb58a, 0x3fdae147, 0xae147af7},	/* -0.571152=f(0.42)*/
{63, 0,123,__LINE__, 0xbfe1bc67, 0x66365026, 0x3fdb851e, 0xb851eb9b},	/* -0.554249=f(0.43)*/
{64, 0,123,__LINE__, 0xbfe134aa, 0x4ccc8542, 0x3fdc28f5, 0xc28f5c3f},	/* -0.537679=f(0.44)*/
{63, 0,123,__LINE__, 0xbfe0af8a, 0xa64cf89f, 0x3fdccccc, 0xcccccce3},	/* -0.521428=f(0.45)*/
{63, 0,123,__LINE__, 0xbfe02ceb, 0xe907006a, 0x3fdd70a3, 0xd70a3d87},	/* -0.505484=f(0.46)*/
{64, 0,123,__LINE__, 0xbfdf5966, 0xeffea91b, 0x3fde147a, 0xe147ae2b},	/* -0.489832=f(0.47)*/
{64, 0,123,__LINE__, 0xbfde5d90, 0xf2270065, 0x3fdeb851, 0xeb851ecf},	/* -0.474461=f(0.48)*/
{63, 0,123,__LINE__, 0xbfdd6627, 0x5ee2acd3, 0x3fdf5c28, 0xf5c28f73},	/* -0.45936=f(0.49)*/
{63, 0,123,__LINE__, 0xbfdc72fe, 0xb3b7b882, 0x3fe00000, 0x0000000b},	/* -0.444519=f(0.5)*/
{63, 0,123,__LINE__, 0xbfdb83ee, 0x28938e2c, 0x3fe051eb, 0x851eb85d},	/* -0.429927=f(0.51)*/
{64, 0,123,__LINE__, 0xbfda98cf, 0x78f2e6a7, 0x3fe0a3d7, 0x0a3d70af},	/* -0.415577=f(0.52)*/
{63, 0,123,__LINE__, 0xbfd9b17e, 0xb2472a4c, 0x3fe0f5c2, 0x8f5c2901},	/* -0.401458=f(0.53)*/
{63, 0,123,__LINE__, 0xbfd8cdda, 0x0702844e, 0x3fe147ae, 0x147ae153},	/* -0.387564=f(0.54)*/
{64, 0,123,__LINE__, 0xbfd7edc1, 0xa5c749e1, 0x3fe19999, 0x999999a5},	/* -0.373887=f(0.55)*/
{63, 0,123,__LINE__, 0xbfd71117, 0x9447ee74, 0x3fe1eb85, 0x1eb851f7},	/* -0.360418=f(0.56)*/
{63, 0,123,__LINE__, 0xbfd637bf, 0x8d72fc1b, 0x3fe23d70, 0xa3d70a49},	/* -0.347153=f(0.57)*/
{64, 0,123,__LINE__, 0xbfd5619e, 0xe292c8ed, 0x3fe28f5c, 0x28f5c29b},	/* -0.334083=f(0.58)*/
{63, 0,123,__LINE__, 0xbfd48e9c, 0x5f133677, 0x3fe2e147, 0xae147aed},	/* -0.321204=f(0.59)*/
{64, 0,123,__LINE__, 0xbfd3bea0, 0x2ea8f036, 0x3fe33333, 0x3333333f},	/* -0.30851=f(0.6)*/
{63, 0,123,__LINE__, 0xbfd2f193, 0xc59d8f70, 0x3fe3851e, 0xb851eb91},	/* -0.295995=f(0.61)*/
{62, 0,123,__LINE__, 0xbfd22761, 0xcb0af396, 0x3fe3d70a, 0x3d70a3e3},	/* -0.283654=f(0.62)*/
{64, 0,123,__LINE__, 0xbfd15ff6, 0x04d6244b, 0x3fe428f5, 0xc28f5c35},	/* -0.271482=f(0.63)*/
{64, 0,123,__LINE__, 0xbfd09b3d, 0x453f55ca, 0x3fe47ae1, 0x47ae1487},	/* -0.259475=f(0.64)*/
{64, 0,123,__LINE__, 0xbfcfb24a, 0xb3c2891b, 0x3fe4cccc, 0xccccccd9},	/* -0.247629=f(0.65)*/
{62, 0,123,__LINE__, 0xbfce3339, 0xf7fc5217, 0x3fe51eb8, 0x51eb852b},	/* -0.235938=f(0.66)*/
{63, 0,123,__LINE__, 0xbfccb927, 0x83f826ca, 0x3fe570a3, 0xd70a3d7d},	/* -0.2244=f(0.67)*/
{63, 0,123,__LINE__, 0xbfcb43f4, 0x23eb43f5, 0x3fe5c28f, 0x5c28f5cf},	/* -0.213011=f(0.68)*/
{64, 0,123,__LINE__, 0xbfc9d382, 0x2a8d95b6, 0x3fe6147a, 0xe147ae21},	/* -0.201767=f(0.69)*/
{63, 0,123,__LINE__, 0xbfc867b5, 0x59ffc6dc, 0x3fe66666, 0x66666673},	/* -0.190665=f(0.7)*/
{63, 0,123,__LINE__, 0xbfc70072, 0xce567b7f, 0x3fe6b851, 0xeb851ec5},	/* -0.179701=f(0.71)*/
{63, 0,123,__LINE__, 0xbfc59da0, 0xe9a74146, 0x3fe70a3d, 0x70a3d717},	/* -0.168873=f(0.72)*/
{63, 0,123,__LINE__, 0xbfc43f27, 0x41772e24, 0x3fe75c28, 0xf5c28f69},	/* -0.158177=f(0.73)*/
{63, 0,123,__LINE__, 0xbfc2e4ee, 0x8d6e34f6, 0x3fe7ae14, 0x7ae147bb},	/* -0.147611=f(0.74)*/
{63, 0,123,__LINE__, 0xbfc18ee0, 0x9734f207, 0x3fe80000, 0x0000000d},	/* -0.137173=f(0.75)*/
{64, 0,123,__LINE__, 0xbfc03ce8, 0x2b6521ad, 0x3fe851eb, 0x851eb85f},	/* -0.126859=f(0.76)*/
{61, 0,123,__LINE__, 0xbfbddde2, 0x16ee44c0, 0x3fe8a3d7, 0x0a3d70b1},	/* -0.116667=f(0.77)*/
{62, 0,123,__LINE__, 0xbfbb49cf, 0xc131b14d, 0x3fe8f5c2, 0x8f5c2903},	/* -0.106595=f(0.78)*/
{64, 0,123,__LINE__, 0xbfb8bd74, 0x5eb81ab8, 0x3fe947ae, 0x147ae155},	/* -0.0966408=f(0.79)*/
{62, 0,123,__LINE__, 0xbfb638ac, 0x9857e6d4, 0x3fe99999, 0x999999a7},	/* -0.0868023=f(0.8)*/
{63, 0,123,__LINE__, 0xbfb3bb56, 0xa0f5dce8, 0x3fe9eb85, 0x1eb851f9},	/* -0.0770773=f(0.81)*/
{62, 0,123,__LINE__, 0xbfb14552, 0x2135f8ee, 0x3fea3d70, 0xa3d70a4b},	/* -0.067464=f(0.82)*/
{63, 0,123,__LINE__, 0xbfadad00, 0x48d1344c, 0x3fea8f5c, 0x28f5c29d},	/* -0.0579605=f(0.83)*/
{62, 0,123,__LINE__, 0xbfa8dd86, 0x0d3a74e3, 0x3feae147, 0xae147aef},	/* -0.0485651=f(0.84)*/
{61, 0,123,__LINE__, 0xbfa41bfc, 0xc78b8379, 0x3feb3333, 0x33333341},	/* -0.039276=f(0.85)*/
{61, 0,123,__LINE__, 0xbf9ed05c, 0x1f5d00d6, 0x3feb851e, 0xb851eb93},	/* -0.0300917=f(0.86)*/
{63, 0,123,__LINE__, 0xbf9583cb, 0x7f59a6c5, 0x3febd70a, 0x3d70a3e5},	/* -0.0210106=f(0.87)*/
{60, 0,123,__LINE__, 0xbf88a3c7, 0x5dfcb6ba, 0x3fec28f5, 0xc28f5c37},	/* -0.0120311=f(0.88)*/
{58, 0,123,__LINE__, 0xbf69d226, 0x276b52f6, 0x3fec7ae1, 0x47ae1489},	/* -0.00315196=f(0.89)*/
{60, 0,123,__LINE__, 0x3f770db5, 0x0ee194be, 0x3feccccc, 0xccccccdb},	/* 0.00562831=f(0.9)*/
{59, 0,123,__LINE__, 0x3f8d4f15, 0x757ba377, 0x3fed1eb8, 0x51eb852d},	/* 0.014311=f(0.91)*/
{61, 0,123,__LINE__, 0x3f977268, 0x67a99a89, 0x3fed70a3, 0xd70a3d7f},	/* 0.0228974=f(0.92)*/
{62, 0,123,__LINE__, 0x3fa0122b, 0xda106704, 0x3fedc28f, 0x5c28f5d1},	/* 0.0313886=f(0.93)*/
{62, 0,123,__LINE__, 0x3fa45ed3, 0x6a0180ba, 0x3fee147a, 0xe147ae23},	/* 0.039786=f(0.94)*/
{61, 0,123,__LINE__, 0x3fa89f50, 0x75b2c68a, 0x3fee6666, 0x66666675},	/* 0.0480905=f(0.95)*/
{61, 0,123,__LINE__, 0x3facd3c7, 0x19481694, 0x3feeb851, 0xeb851ec7},	/* 0.0563032=f(0.96)*/
{64, 0,123,__LINE__, 0x3fb07e2d, 0x0606b9d8, 0x3fef0a3d, 0x70a3d719},	/* 0.0644253=f(0.97)*/
{62, 0,123,__LINE__, 0x3fb28c95, 0x5852b1f5, 0x3fef5c28, 0xf5c28f6b},	/* 0.0724576=f(0.98)*/
{62, 0,123,__LINE__, 0x3fb4952c, 0x92323b15, 0x3fefae14, 0x7ae147bd},	/* 0.0804012=f(0.99)*/
{62, 0,123,__LINE__, 0x3fb69802, 0x26f35937, 0x3ff00000, 0x00000007},	/* 0.088257=f(1)*/
{63, 0,123,__LINE__, 0x3fb89524, 0xf5756984, 0x3ff028f5, 0xc28f5c30},	/* 0.0960258=f(1.01)*/
{62, 0,123,__LINE__, 0x3fba8ca3, 0x4ebf2955, 0x3ff051eb, 0x851eb859},	/* 0.103708=f(1.02)*/
{62, 0,123,__LINE__, 0x3fbc7e8a, 0xfc424cfe, 0x3ff07ae1, 0x47ae1482},	/* 0.111306=f(1.03)*/
{62, 0,123,__LINE__, 0x3fbe6ae9, 0x45d171b9, 0x3ff0a3d7, 0x0a3d70ab},	/* 0.118819=f(1.04)*/
{63, 0,123,__LINE__, 0x3fc028e5, 0x7ba671d6, 0x3ff0cccc, 0xccccccd4},	/* 0.126248=f(1.05)*/
{63, 0,123,__LINE__, 0x3fc1199e, 0x3305325a, 0x3ff0f5c2, 0x8f5c28fd},	/* 0.133594=f(1.06)*/
{63, 0,123,__LINE__, 0x3fc207a4, 0xbafdecff, 0x3ff11eb8, 0x51eb8526},	/* 0.140858=f(1.07)*/
{63, 0,123,__LINE__, 0x3fc2f2fe, 0xcf4c4294, 0x3ff147ae, 0x147ae14f},	/* 0.148041=f(1.08)*/
{62, 0,123,__LINE__, 0x3fc3dbb1, 0xf7c61893, 0x3ff170a3, 0xd70a3d78},	/* 0.155142=f(1.09)*/
{62, 0,123,__LINE__, 0x3fc4c1c3, 0x8a97c193, 0x3ff19999, 0x999999a1},	/* 0.162163=f(1.1)*/
{62, 0,123,__LINE__, 0x3fc5a538, 0xae6635dc, 0x3ff1c28f, 0x5c28f5ca},	/* 0.169105=f(1.11)*/
{62, 0,123,__LINE__, 0x3fc68616, 0x5c58c206, 0x3ff1eb85, 0x1eb851f3},	/* 0.175967=f(1.12)*/
{62, 0,123,__LINE__, 0x3fc76461, 0x620b7b62, 0x3ff2147a, 0xe147ae1c},	/* 0.182751=f(1.13)*/
{63, 0,123,__LINE__, 0x3fc8401e, 0x636bb4c7, 0x3ff23d70, 0xa3d70a45},	/* 0.189457=f(1.14)*/
{62, 0,123,__LINE__, 0x3fc91951, 0xdc7f9b62, 0x3ff26666, 0x6666666e},	/* 0.196085=f(1.15)*/
{62, 0,123,__LINE__, 0x3fc9f000, 0x231a10d0, 0x3ff28f5c, 0x28f5c297},	/* 0.202637=f(1.16)*/
{62, 0,123,__LINE__, 0x3fcac42d, 0x687bc739, 0x3ff2b851, 0xeb851ec0},	/* 0.209112=f(1.17)*/
{62, 0,123,__LINE__, 0x3fcb95dd, 0xbae2947f, 0x3ff2e147, 0xae147ae9},	/* 0.215511=f(1.18)*/
{62, 0,123,__LINE__, 0x3fcc6515, 0x0707e2ea, 0x3ff30a3d, 0x70a3d712},	/* 0.221835=f(1.19)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc01921fb, 0x54442d18},	/* nan=f(-6.28319)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc012d97c, 0x7f3321d2},	/* nan=f(-4.71239)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc00921fb, 0x54442d18},	/* nan=f(-3.14159)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff921fb, 0x54442d18},	/* nan=f(-1.5708)*/
{ 0, 0, 34,__LINE__, 0xfff00000, 0x00000000, 0x00000000, 0x00000000},	/* -inf=f(0)*/
{62, 0,123,__LINE__, 0x3fda3d7f, 0xedadd040, 0x3ff921fb, 0x54442d18},	/* 0.410004=f(1.5708)*/
{61, 0,123,__LINE__, 0x3fd503f4, 0x1f0be44e, 0x400921fb, 0x54442d18},	/* 0.328366=f(3.14159)*/
{59, 0,123,__LINE__, 0xbfd02737, 0x6bc5f936, 0x4012d97c, 0x7f3321d2},	/* -0.252394=f(4.71239)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc03e0000, 0x00000000},	/* nan=f(-30)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc03c4ccc, 0xcccccccd},	/* nan=f(-28.3)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc03a9999, 0x9999999a},	/* nan=f(-26.6)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc038e666, 0x66666667},	/* nan=f(-24.9)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0373333, 0x33333334},	/* nan=f(-23.2)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0358000, 0x00000001},	/* nan=f(-21.5)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc033cccc, 0xccccccce},	/* nan=f(-19.8)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0321999, 0x9999999b},	/* nan=f(-18.1)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0306666, 0x66666668},	/* nan=f(-16.4)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc02d6666, 0x6666666a},	/* nan=f(-14.7)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc02a0000, 0x00000004},	/* nan=f(-13)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0269999, 0x9999999e},	/* nan=f(-11.3)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0233333, 0x33333338},	/* nan=f(-9.6)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc01f9999, 0x999999a3},	/* nan=f(-7.9)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc018cccc, 0xccccccd6},	/* nan=f(-6.2)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0120000, 0x00000009},	/* nan=f(-4.5)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xc0066666, 0x66666678},	/* nan=f(-2.8)*/
{64, 0, 33,__LINE__, 0x7ff80000, 0x00000000, 0xbff19999, 0x999999bd},	/* nan=f(-1.1)*/
{64, 0,123,__LINE__, 0xbfd3bea0, 0x2ea8f107, 0x3fe33333, 0x333332ec},	/* -0.30851=f(0.6)*/
{63, 0,123,__LINE__, 0x3fe09412, 0xda725e0e, 0x40026666, 0x66666654},	/* 0.518075=f(2.3)*/
{57, 0,123,__LINE__, 0xbf9158e9, 0xc57c1c47, 0x400fffff, 0xffffffee},	/* -0.0169407=f(4)*/
{58, 0,123,__LINE__, 0xbfd50086, 0xceea168e, 0x4016cccc, 0xccccccc4},	/* -0.328157=f(5.7)*/
{55, 0,123,__LINE__, 0x3fb736dc, 0xb6bee6ee, 0x401d9999, 0x99999991},	/* 0.0906809=f(7.4)*/
{61, 0,123,__LINE__, 0x3fce81cb, 0x35aa7ce5, 0x40223333, 0x3333332f},	/* 0.238336=f(9.1)*/
{60, 0,123,__LINE__, 0xbfc0fa4b, 0x699333dd, 0x40259999, 0x99999995},	/* -0.132638=f(10.8)*/
{61, 0,123,__LINE__, 0xbfc5ea59, 0xb440a964, 0x4028ffff, 0xfffffffb},	/* -0.171214=f(12.5)*/
{61, 0,123,__LINE__, 0x3fc42a56, 0xd5029d90, 0x402c6666, 0x66666661},	/* 0.157542=f(14.2)*/
{59, 0,123,__LINE__, 0x3fbcf7b9, 0x4cdab6b8, 0x402fcccc, 0xccccccc7},	/* 0.113155=f(15.9)*/
{64, 0,123,__LINE__, 0xbfc5afb0, 0xfa559fa9, 0x40319999, 0x99999997},	/* -0.169424=f(17.6)*/
{62, 0,123,__LINE__, 0xbfaf2149, 0x60705431, 0x40334ccc, 0xccccccca},	/* -0.0608008=f(19.3)*/
{63, 0,123,__LINE__, 0x3fc5c92b, 0xd5128a72, 0x4034ffff, 0xfffffffd},	/* 0.170202=f(21)*/
{63, 0,123,__LINE__, 0x3f8c1ae3, 0xf028918f, 0x4036b333, 0x33333330},	/* 0.0137232=f(22.7)*/
{64, 0,123,__LINE__, 0xbfc4a662, 0x4701d3a0, 0x40386666, 0x66666663},	/* -0.161328=f(24.4)*/
{62, 0,123,__LINE__, 0x3f9c2b29, 0x4eeab909, 0x403a1999, 0x99999996},	/* 0.0275084=f(26.1)*/
{64, 0,123,__LINE__, 0x3fc2771c, 0x9fef6e9f, 0x403bcccc, 0xccccccc9},	/* 0.14426=f(27.8)*/
{61, 0,123,__LINE__, 0xbfafc3a0, 0x56b6c329, 0x403d7fff, 0xfffffffc},	/* -0.0620394=f(29.5)*/
0,};
void test_y0(m)   {run_vector_1(m,y0_vec,(char *)(y0),"y0","dd");   }	
