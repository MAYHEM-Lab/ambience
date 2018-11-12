//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#include <stdio.hpp>
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
            return buf.slice(0, ::std::cin.readsome(buf.data(), buf.size()));
        }
    } // namespace x86
} // namespace tos