//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#include <tos/print.hpp>
#include <stdlib.h>

static char * itoa(int i, int base) {
    static char intbuf[12];
    static char lookup[] = {\
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };
    int j = 0, isneg = 0;

    if (i == 0) {
        intbuf[0] = '0';
        intbuf[1] = '\0';
        return intbuf;
    }

    if (i < 0) {
        isneg = 1;
        i = -i;
    }

    while (i != 0) {
        intbuf[j++] = lookup[(i % base)];
        i /= base;
    }

    if (isneg)
        intbuf[j++] = '-';

    intbuf[j] = '\0';
    j--;
    i = 0;
    while (i < j) {
        isneg = intbuf[i];
        intbuf[i] = intbuf[j];
        intbuf[j] = isneg;
        i++;
        j--;
    }

    return intbuf;
}

namespace tos
{
    void print(char_ostream & ostr, const char *str) {
        while (*str)
        {
            print(ostr, *str++);
        }
    }

    void print(char_ostream &ostr, bool b) {
        print(ostr, b ? "true" : "false");
    }

    void print(char_ostream & ostr, int16_t x) {
        print(ostr, itoa(x, 10));
    }

    void print(char_ostream & ostr, int32_t x) {
        print(ostr, itoa(x, 10));
    }

    void print(char_ostream & ostr, void *p) {
        print(ostr, reinterpret_cast<uintptr_t>(p));
    }

    void print(char_ostream & ostr, uintptr_t ptr) {
        print(ostr, '0');
        print(ostr, 'x');
        print(ostr, itoa(ptr, 16));
    }
}