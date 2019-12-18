//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#include <arch/stdio.hpp>
#include <iostream>

namespace tos {
namespace x86 {
int stdio::write(span<const uint8_t> buf) {
    ::std::cout.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    return buf.size();
}

span<uint8_t> stdio::read(span<uint8_t> buf) {
    ::std::cin.read(reinterpret_cast<char*>(buf.data()), buf.size());
    return buf;
}
} // namespace x86
} // namespace tos