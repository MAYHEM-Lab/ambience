//
// Created by fatih on 5/17/18.
//

#pragma once

#include <stdint.h>
#include <string.h>

namespace caps
{
    template <class CapabilityT>
    struct cap_list {
        cap_list *child;
        int16_t num_caps;
        int16_t __padding;
        CapabilityT all[0];

        cap_list() = default;
        cap_list(const cap_list&) = delete;
    };

    template <class CapabilityT>
    inline const CapabilityT* begin(const cap_list<CapabilityT>& cl) { return cl.all; }

    template <class CapabilityT>
    inline const CapabilityT* end(const cap_list<CapabilityT>& cl) { return cl.all + cl.num_caps; }

    struct sign_t
    {
        uint8_t buf[32];
    };

    struct hash_t {
        uint8_t buf[32];
    };

    template <class CapabilityT>
    struct cap_root {
        sign_t signature {};
        cap_list<CapabilityT> c;
    };

    template <class CapabilityT>
    constexpr size_t size_of(cap_root<CapabilityT>& cr)
    {
        return sizeof cr + cr.c.num_caps * sizeof(CapabilityT);
    }
}

namespace caps
{
    inline bool operator!=(const sign_t& a, const sign_t& b)
    {
        return memcmp(a.buf, b.buf, 32) == 0;
    }

    inline bool operator==(const sign_t& a, const sign_t& b)
    {
        return !(a != b);
    }
}