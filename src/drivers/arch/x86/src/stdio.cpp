//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#include <arch/x86/stdio.hpp>
#include <iostream>

namespace tos
{
    namespace x86
    {
        int stdio::write(span<const char> buf)
        {
            ::std::cout.write(buf.data(), buf.size());
            return buf.size();
        }

        span<char> stdio::read(span<char> buf)
        {
            ::std::cin.read(buf.data(), buf.size());
            return buf;
        }
    } // namespace x86
} // namespace tos