//
// Created by fatih on 5/21/19.
//

#pragma once

#include <tos/print.hpp>

namespace tos
{
namespace ble
{
struct address_t
{
    uint8_t addr[6];
};

template <class StreamT>
void print(StreamT& to, const address_t& addr)
{
    for (auto p : addr.addr)
    {
        print(to, size_t(p), "");
    }
}
} // namespace ble

} //namespace tos