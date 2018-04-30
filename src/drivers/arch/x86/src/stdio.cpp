//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#include <stdio.hpp>
#include <iostream>

namespace tos
{
    namespace x86
    {
        void stdio::write(const char* data, int len)
        {
            std::cout.write(data, len);
        }

        int stdio::read(char* data, int len)
        {
            return std::cin.readsome(data, len);
        }

        void stdio::putc(char c)
        {
            std::cout << c;
        }
    }
}