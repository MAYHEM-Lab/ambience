#include <stdarg.h>
#include <stdio.h>
#include <tos/debug/log.hpp>

extern "C" {
int printf(const char* fmt, ...) {
    static char buf[1024];

    va_list myargs;
    va_start(myargs, fmt);
    auto ret = vsnprintf(buf, std::size(buf), fmt, myargs);
    tos::debug::log(buf);
    va_end(myargs);

    return ret;
}
}