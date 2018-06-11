//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#include <stdio.hpp>
#include <iostream>

namespace tos
{
    namespace x86
    {
        void stdio::write(span<const char> buf)
        {
            std::cout.write(buf.data(), buf.size());
        }

        int stdio::read(span<char> buf)
        {
            return std::cin.readsome(buf.data(), buf.size());
        }

        void stdio::putc(char c)
        {
            std::cout << c;
        }
    }
}