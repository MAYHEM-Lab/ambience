//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#include <tos/print.hpp>
#include <stdlib.h>

namespace tos
{
    const char* itoa(int32_t i, int base) {
        static char intbuf[12];
        static char lookup[] = {\
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
        };
        int j = 0, isneg = 0;

        if (i == 0) {
            return "0";
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
}