//
// Created by fatih on 7/12/18.
//

#pragma once

extern "C" int ets_printf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
#define tos_debug_print ets_printf