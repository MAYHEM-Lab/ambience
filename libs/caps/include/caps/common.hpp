//
// Created by fatih on 5/17/18.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <tos/span.hpp>
#include <tos/utility.hpp>

namespace caps {
struct raw_deleter {
    template<class T>
    void operator()(T* ptr) const {
        delete[](char*) ptr;
    }
};

template<class CapabilityT>
struct cap_list;

template<class CapT>
using list_ptr = std::unique_ptr<cap_list<CapT>, raw_deleter>;

/**
 * This struct template represents a single node in a capability derivation chain.
 * @tparam CapabilityT type of the capabilities stored in this node.
 */
template<class CapabilityT>
struct cap_list : tos::non_copy_movable {
    static_assert(std::is_trivially_copyable_v<CapabilityT>);

    list_ptr<CapabilityT> child;
    int16_t num_caps;
    int16_t __padding;
    CapabilityT all[0];

    tos::span<const CapabilityT> span() const {
        return tos::span<const CapabilityT>(all, num_caps);
    }

    tos::span<CapabilityT> span() {
        return tos::span<CapabilityT>(all, num_caps);
    }
};

template<class T>
list_ptr<T> clone(const cap_list<T>& from) {
    auto mem = new char[sizeof(cap_list<T>) + from.num_caps * sizeof(T)];
    auto res = new (mem) cap_list<T>;
    std::uninitialized_copy(from.span().begin(), from.span().end(), res->span().begin());
    res->num_caps = from.num_caps;
    res->child = from.child ? clone(*from.child) : nullptr;
    return list_ptr<T>(res);
}

/**
 * This struct template represents the root of a capability token.
 *
 * @tparam CapabilityT an application defined type for storing authorization information.
 * @tparam CryptoModelT the cryptographic model for signing and verifying signatures.
 */
template<class CapabilityT, class CryptoModelT>
struct token {
    using sign_t = typename CryptoModelT::sign_type;

    sign_t signature{};
    cap_list<CapabilityT> root;
};

template<class CapT, class CryptoModelT>
using token_ptr = std::unique_ptr<token<CapT, CryptoModelT>, raw_deleter>;

template<class CT, class ST>
token_ptr<CT, ST> clone(const token<CT, ST>& from) {
    auto mem = new char[sizeof(token<CT, ST>) + sizeof(CT) * from.root.num_caps];
    auto result = new (mem) token<CT, ST>;
    std::uninitialized_copy(
        from.root.span().begin(), from.root.span().end(), result->root.span().begin());
    result->root.num_caps = from.root.num_caps;
    result->signature = from.signature;
    result->root.child = result->root.child ? clone(*result->root.child) : nullptr;
    return token_ptr<CT, ST>(result);
}

/**
 * Returns the size of the given capability token in bytes.
 * Useful as the type uses a variable length array.
 */
template<class CapabilityT, class CryptoModelT>
constexpr size_t size_of(const token<CapabilityT, CryptoModelT>& cr) {
    return sizeof cr + cr.root.num_caps * sizeof(CapabilityT);
}
} // namespace caps