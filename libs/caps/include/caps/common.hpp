//
// Created by fatih on 5/17/18.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <memory>

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

    template <class CapabilityT, class SignerT>
    struct token {
        using sign_t = typename SignerT::sign_t;

        sign_t signature {};
        cap_list<CapabilityT> c;
    };

    template <class CapabilityT, class SignerT>
    constexpr size_t size_of(token<CapabilityT, SignerT>& cr)
    {
        return sizeof cr + cr.c.num_caps * sizeof(CapabilityT);
    }

    struct raw_deleter
    {
        template <class T>
        void operator()(T* ptr) const
        {
            delete[] (char*)ptr;
        }
    };

    template <class CapT, class SignT>
    using token_ptr = std::unique_ptr<token<CapT, SignT>, raw_deleter>;

    template <class CapT>
    using list_ptr = std::unique_ptr<cap_list<CapT>, raw_deleter>;

}