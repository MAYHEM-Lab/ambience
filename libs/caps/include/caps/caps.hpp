//
// Created by fatih on 10/19/18.
//

#pragma once

#include "common.hpp"
#include "signer.hpp"
#include <string.h>
#include <initializer_list>
#include <new>
#include <memory>
#include <algorithm>

namespace caps
{
    template <class CapabilityT>
    caps::sign_t sign(const caps::cap_root<CapabilityT> &c, caps::signer &s);

    template <class CapabilityT>
    caps::hash_t hash(const caps::cap_list<CapabilityT> &c);

    template <class CapabilityT>
    bool verify(const caps::cap_root<CapabilityT> &c, caps::signer &s);

    template <class CapabilityT, class SatisfyCheckerT>
    inline bool validate(const caps::cap_root<CapabilityT>& cr, const CapabilityT& needle, SatisfyCheckerT&& satisfies);

    template <class CapabilityT>
    void attach(caps::cap_root<CapabilityT> &c, caps::cap_list<CapabilityT> &child);

    template <class StreamT, class CapabilityT>
    void serialize(StreamT& to, const caps::cap_root<CapabilityT>& caps);

    template <class StreamT, class CapabilityT>
    auto deserialize(StreamT& from);
}

namespace caps
{
    template<class CapabilityT, class SatisfyCheckerT>
    bool validate(
            const caps::cap_root<CapabilityT> &haystack,
            const CapabilityT &needle,
            SatisfyCheckerT &&satisfies) {
        auto* leaf = &haystack.c;
        for (; leaf->child; leaf = leaf->child); // find the end of the hmac chain

        return std::any_of(begin(*leaf), end(*leaf), [&needle, &satisfies](auto& cap){
            return satisfies(needle, cap);
        });
    }

    template <class CapabilityT>
    inline caps::sign_t sign(const caps::cap_root<CapabilityT> &c, caps::signer &s)
    {
        caps::sign_t res{};
        auto beg = (const uint8_t *) c.c.all;
        size_t sz = sizeof(CapabilityT) * c.c.num_caps;
        s.sign({beg, sz}, res.buf);
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

    struct raw_deleter
    {
        template <class T>
        void operator()(T* ptr) const
        {
            delete[] (char*)ptr;
        }
    };

    template <class CapabilityT>
    std::unique_ptr<caps::cap_root<CapabilityT>, raw_deleter> mkcaps(std::initializer_list<CapabilityT> capabs, caps::signer &s)
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
        return std::unique_ptr<caps::cap_root<CapabilityT>, raw_deleter>{cps};
    }

    namespace detail
    {
        template <class StreamT>
        void serialize(StreamT& to, const sign_t& signature)
        {
            static_assert(std::is_pod<sign_t>{}, "signature must be a pod");
            to.write({ (const char*)&signature, sizeof signature });
        }

        template <class StreamT, class CapabilityT>
        void serialize(StreamT& to, const cap_list<CapabilityT>& list)
        {
            to.write({ (const char*)&list.num_caps, sizeof list.num_caps });
            for (auto& cap : list)
            {
                static_assert(std::is_pod<decltype(cap)>{}, "capabilities must be pods");
                to.write({ (const char*)&cap, sizeof cap });
            }
        }

        template <class StreamT>
        sign_t deserialize_sign(StreamT& from)
        {
            sign_t signature;
            from.read({ (char*)&signature, sizeof signature });
            return signature;
        }

        template <class CapabilityT, class StreamT>
        void read_list(StreamT& from, cap_list<CapabilityT>& to, int16_t len)
        {
            for (int i = 0; i < len; ++i)
            {
                from.read({ (char*)to.all[i], sizeof to.all[i] });
            }
            to.num_caps = len;
        }

        template <class CapabilityT, class StreamT>
        std::unique_ptr<cap_root<CapabilityT>, raw_deleter> deserialize_root(StreamT& from)
        {
            decltype(cap_list<CapabilityT>::num_caps) len;
            from.read({ (char*)&len, sizeof len });
            auto mem = new char[sizeof(caps::cap_root<CapabilityT>) + sizeof(CapabilityT) * len];
            auto cps = new(mem) caps::cap_root<CapabilityT>;
            read_list(from, cps->c, len);
            return std::unique_ptr<caps::cap_root<CapabilityT>, raw_deleter>{cps};
        }

        template <class CapabilityT, class StreamT>
        std::unique_ptr<cap_list<CapabilityT>, raw_deleter> deserialize_list(StreamT& from)
        {
            decltype(cap_list<CapabilityT>::num_caps) len;
            from.read({ (char*)&len, sizeof len });
            if (len == 0) return nullptr;
            auto mem = new char[sizeof(caps::cap_list<CapabilityT>) + sizeof(CapabilityT) * len];
            auto cps = new(mem) caps::cap_list<CapabilityT>;
            read_list(from, *cps, len);
            return std::unique_ptr<caps::cap_list<CapabilityT>, raw_deleter>{cps};
        }
    }

    template<class StreamT, class CapabilityT>
    void serialize(StreamT& to, const cap_root<CapabilityT>& caps)
    {
        for (auto child = &caps.c; child; child = child->child)
        {
            detail::serialize(to, *child);
        }
        char c[] = {0};
        to.write(c);
        detail::serialize(to, caps.signature);
    }

    template<class StreamT, class CapabilityT>
    auto deserialize(StreamT& from)
    {
        auto root = detail::deserialize_root(from);
        auto tail = &root->c;
        for (auto l = detail::deserialize_list(from); l; l = detail::deserialize_list(from))
        {
            tail->child = l.release();
            tail = tail->child;
        }
        tail->child = nullptr;
        root->signature = detail::deserialize_sign(from);
        return root;
    }
}