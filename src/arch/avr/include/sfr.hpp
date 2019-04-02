//
// Created by fatih on 4/1/19.
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace tos
{
    template <class SfrDef>
    struct reg
    {
    public:
        void write(uint8_t val)
        {
            get() = val;
        }

        uint8_t read() const
        {
            return get();
        }

    private:
        auto& get()
        {
            return *reinterpret_cast<volatile uint8_t*>(SfrDef::address);
        }
    };
} // namespace tos