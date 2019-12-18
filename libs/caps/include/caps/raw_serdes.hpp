//
// Created by fatih on 1/8/19.
//

#pragma once

#include "common.hpp"

#include <tos/streams.hpp>
#include <type_traits>

namespace caps {
template<class StreamT, class SignerT, class CapabilityT>
void serialize(StreamT& to, const token<CapabilityT, SignerT>& caps);

template<class CapabilityT, class SignerT, class StreamT>
auto deserialize(StreamT& from);
} // namespace caps

namespace caps {
namespace detail {
template<class SignT, class StreamT>
void serialize(StreamT& to, const SignT& signature) {
    static_assert(std::is_trivially_copyable<SignT>{}, "signature must be a pod");
    to->write({reinterpret_cast<const uint8_t*>(&signature), sizeof signature});
}

template<class StreamT, class CapabilityT>
void serialize(StreamT& to, const cap_list<CapabilityT>& list) {
    to->write({reinterpret_cast<const uint8_t*>(&list.num_caps), sizeof list.num_caps});
    for (auto& cap : list.span()) {
        static_assert(
            std::is_trivially_copyable<std::remove_reference_t<decltype(cap)>>{},
            "capabilities must be pods");
        to->write({reinterpret_cast<const uint8_t*>(&cap), sizeof cap});
    }
}

template<class SignT, class StreamT>
SignT deserialize_sign(StreamT& from) {
    static_assert(std::is_trivially_copyable<SignT>{}, "signature must be a pod");

    SignT signature;
    tos::read_to_end(from, {(uint8_t*)&signature, sizeof signature});
    return signature;
}

template<class CapabilityT, class StreamT>
void read_list(StreamT& from, cap_list<CapabilityT>& to, int16_t len) {
    for (int i = 0; i < len; ++i) {
        tos::read_to_end(from, {(uint8_t*)&to.all[i], sizeof to.all[i]});
    }
    to.num_caps = len;
}

template<class CapabilityT, class CryptoModelT, class StreamT>
token_ptr<CapabilityT, CryptoModelT> deserialize_root(StreamT& from) {
    decltype(cap_list<CapabilityT>::num_caps) len;
    tos::read_to_end(from, {(uint8_t*)&len, sizeof len});
    tos_debug_print("len: %d\n", int(len));
    auto mem =
        new uint8_t[sizeof(token<CapabilityT, CryptoModelT>) + sizeof(CapabilityT) * len];
    auto cps = new (mem) token<CapabilityT, CryptoModelT>;
    read_list(from, cps->root, len);
    return token_ptr<CapabilityT, CryptoModelT>{cps};
}

template<class CapabilityT, class StreamT>
list_ptr<CapabilityT> deserialize_list(StreamT& from) {
    decltype(cap_list<CapabilityT>::num_caps) len;
    tos::read_to_end(from, {(uint8_t*)&len, sizeof len});
    if (len == 0)
        return nullptr;
    auto mem =
        new uint8_t[sizeof(caps::cap_list<CapabilityT>) + sizeof(CapabilityT) * len];
    auto cps = new (mem) caps::cap_list<CapabilityT>;
    read_list(from, *cps, len);
    return list_ptr<CapabilityT>{cps};
}
} // namespace detail

template<class StreamT, class SignerT, class CapabilityT>
void serialize(StreamT& to, const token<CapabilityT, SignerT>& caps) {
    // Serialize all the links in the capability chain.
    for (auto node = &caps.root; node; node = node->child.get()) {
        detail::serialize(to, *node);
    }

    decltype(caps.root.num_caps) c[] = {0};
    to->write({reinterpret_cast<const uint8_t*>(&c), sizeof c});

    /*
     * The signature goes last into the wire.
     *
     * If it went first, since the receiver couldn't know the size of the
     * overall object, it'd have to store the signature in a temporary location
     * probably on the stack. Signatures could be huge, so we prefer not to
     * store a temporary signature and then copy it.
     *
     * When the signature goes last, the receiver can actually construct the
     * whole capability chain and just copy the signature to the signature
     * field of the root. This way, we save stack space and avoid copying
     * large signatures.
     */
    detail::serialize(to, caps.signature);
}

template<class CapabilityT, class CryptoModelT, class StreamT>
auto deserialize(StreamT& from) {
    auto root = detail::deserialize_root<CapabilityT, CryptoModelT>(from);
    auto tail = &root->root;
    for (auto l = detail::deserialize_list<CapabilityT>(from); l;
         l = detail::deserialize_list<CapabilityT>(from)) {
        tail->child = std::move(l);
        tail = tail->child.get();
    }
    tail->child = nullptr;
    root->signature = detail::deserialize_sign<typename CryptoModelT::sign_type>(from);
    return root;
}
} // namespace caps