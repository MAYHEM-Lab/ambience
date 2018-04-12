//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#pragma once

#include <tos/char_stream.hpp>
#include <stdint.h>
#include <tos/utility.hpp>

namespace tos
{
    /**
     * Prints the given null terminated string to the output stream
     * @param ostr The output stream
     * @param str A null terminated string
     */
    void print(char_ostream& ostr, const char* str);

    inline void print(char_ostream& ostr, char c) {
        ostr.putc(c);
    }

    void print(char_ostream& ostr, int32_t i);
    void print(char_ostream& ostr, int16_t i);

    void print(char_ostream& ostr, bool b);

    void print(char_ostream& ostr, uintptr_t);
    void print(char_ostream& ostr, void* p);

    /**
     * Prints the given arguments with the given separator in between
     *
     * To prevent overload ambiguities, this function takes 2 regular arguments
     * and then a variadic pack of arguments
     *
     * @tparam T1 Type of the first parameter
     * @tparam T2 Type of the second parameter
     * @tparam Ts Types of the rest of the parameters
     * @param ostr Stream to print the output to
     * @param t1 First parameter
     * @param t2 Second parameter
     * @param ts Rest of the parameters
     * @param sep Separator
     */
    template<class T1, class T2, class... Ts>
    void print(char_ostream& ostr, T1&& t1, T2&& t2, Ts&&... ts, char sep = ' ')
    {
        print(ostr, forward<T1>(t1));
        print(ostr, sep);
        print(ostr, forward<T2>(t2));

        //((print(ostr, sep), print(ostr, forward<Ts>(ts))), ...);
        int _[] = { (print(ostr, sep), print(ostr, forward<Ts>(ts)), 0)... };
        (void)_;
    }

    template <class... T>
    void println(char_ostream& ostr, T&&... t)
    {
        print(ostr, forward<T>(t)...);
        print(ostr, '\n');
    }
}

