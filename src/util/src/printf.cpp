#include <stdarg.h>
#include <stdio.h>
#include <tos/debug/log.hpp>

extern "C" {
int printf(const char* fmt, ...) {
    static char buf[1024];

    va_list myargs;
    va_start(myargs, fmt);
    auto ret = vsnprintf(buf, std::size(buf), fmt, myargs);
    auto span = tos::span(buf).slice(0, ret);
     while (span.back() == '\0') {
        span = span.pop_back();
    }
    while (span.back() == '\n') {
        span = span.pop_back();
    }
    tos::debug::log(std::string_view(span.begin(), span.size()));
    va_end(myargs);

    return ret;
}
}