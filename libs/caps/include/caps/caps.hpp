//
// Created by fatih on 10/19/18.
//

#pragma once

#include "common.hpp"
#include "signer.hpp"
#include <string.h>
#include <initializer_list>
#include <tos/new.hpp>

namespace caps
{
    template <class CapabilityT>
    caps::sign_t sign(const caps::cap_root<CapabilityT> &c, caps::signer &s);

    template <class CapabilityT>
    caps::hash_t hash(const caps::cap_list<CapabilityT> &c);

    template <class CapabilityT>
    bool verify(const caps::cap_root<CapabilityT> &c, caps::signer &s);

    template <class CapabilityT, class SatisfyCheckerT>
    inline bool validate(const caps::cap_root<CapabilityT>& cr, const CapabilityT& sub, SatisfyCheckerT&& satisfies);

    template <class CapabilityT>
    void attach(caps::cap_root<CapabilityT> &c, caps::cap_list<CapabilityT> &child);
}

namespace caps
{
    template< class InputIt, class UnaryPredicate >
    constexpr bool any_of(InputIt first, InputIt last, UnaryPredicate p)
    {
        for (; first != last; ++first)
        {
            if (p(*first))
            {
                return true;
            }
        }
        return false;
    }

    template<class CapabilityT, class SatisfyCheckerT>
    bool validate(
            const caps::cap_root<CapabilityT> &cr,
            const CapabilityT &sub,
            SatisfyCheckerT &&satisfies) {
        auto* leaf = &cr.c;
        for (; leaf->child; leaf = leaf->child); // find the end of the hmac chain

        return any_of(begin(*leaf), end(*leaf), [&sub, &satisfies](auto& cap){
            return satisfies(sub, cap);
        });
    }

    template <class CapabilityT>
    inline caps::sign_t sign(const caps::cap_root<CapabilityT> &c, caps::signer &s)
    {
        caps::sign_t res{};
        auto beg = (const uint8_t *) c.c.all;
        size_t sz = sizeof(CapabilityT) * c.c.num_caps;
        uint8_t sig[emsha::SHA256_HASH_SIZE];
        s.sign({beg, sz}, sig);
        memcpy(res.buf, sig, emsha::SHA256_HASH_SIZE);
        return res;
    }

    template <class CapabilityT>
    inline caps::hash_t hash(const caps::cap_list<CapabilityT> &c)
    {
        caps::hash_t res{};
        const auto beg = (const uint8_t *) c.all;
        const auto sz = sizeof(CapabilityT) * c.num_caps;
        emsha::sha256_digest(beg, sz, (uint8_t*)res.buf);
        return res;
    }

    inline void merge_into(sign_t& s, const hash_t& h)
    {
        auto i = s.buf;
        auto i_end = s.buf + 32;
        auto j = h.buf;
        for (; i != i_end; ++i, ++j)
        {
            *i ^= *j;
        }
        emsha::sha256_digest(s.buf, 32, s.buf);
    }

    template <class CapabilityT>
    inline cap_list<CapabilityT>* get_leaf_cap(caps::cap_root<CapabilityT>& c)
    {
        auto it = &c.c;
        while (it->child)
        {
            it = it->child;
        }
        return it;
    }

    template <class CapabilityT>
    inline void attach(caps::cap_root<CapabilityT> &c, caps::cap_list<CapabilityT> &child)
    {
        //TODO: do verification here
        auto child_hash = hash(child);
        merge_into(c.signature, child_hash);
        get_leaf_cap(c)->child = &child;
    }

    template <class CapabilityT>
    inline bool verify(const caps::cap_root<CapabilityT> &c, caps::signer &s)
    {
        caps::sign_t signature = sign(c, s);

        for (auto child = c.c.child; child; child = child->child)
        {
            auto child_hash = hash(*child);
            merge_into(signature, child_hash);
        }

        return signature == c.signature;
    }

    template <class CapabilityT>
    caps::cap_list<CapabilityT>* mkcaps(std::initializer_list<CapabilityT> capabs)
    {
        auto mem = new char[sizeof(caps::cap_list<CapabilityT>) + sizeof(CapabilityT) * capabs.size()];
        auto cps = new(mem) caps::cap_list<CapabilityT>;
        int i = 0;
        for (auto &c : capabs) {
            new(cps->all + i++) CapabilityT(c);
        }
        cps->num_caps = i;
        cps->child = nullptr;
        return cps;
    }

    template <class CapabilityT>
    caps::cap_root<CapabilityT>* mkcaps(std::initializer_list<CapabilityT> capabs, caps::signer &s)
    {
        auto mem = new char[sizeof(caps::cap_root<CapabilityT>) + sizeof(CapabilityT) * capabs.size()];
        auto cps = new(mem) caps::cap_root<CapabilityT>;
        int i = 0;
        for (auto &c : capabs) {
            new(cps->c.all + i++) CapabilityT(c);
        }
        cps->c.num_caps = i;
        cps->c.child = nullptr;
        cps->signature = sign(*cps, s);
        return cps;
    }
}