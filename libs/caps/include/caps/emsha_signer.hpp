//
// Created by fatih on 10/19/18.
//

#pragma once

#include <cstring>
#include <emsha/hmac.hh>
#include <tos/span.hpp>
#include <caps/sha2_generated.h>
#include <tos/utility.hpp>

namespace caps {
namespace sha2 {
inline bool operator==(const Signature& a, const Signature& b) {
    return std::equal(a.signature()->begin(), a.signature()->end(), b.signature()->begin());
}
}

namespace emsha {
struct hash_t {
    uint8_t buf[32];
};

using sign_t = caps::sha2::Signature;

inline bool operator==(const hash_t& h, const hash_t& b) {
    return memcmp(h.buf, b.buf, 32) == 0;
}

void merge_into(sign_t& s, const hash_t& h);

struct hasher {
    hash_t hash(tos::span<const uint8_t> msg) const {
        hash_t h;
        ::emsha::sha256_digest(msg.data(), msg.size(), h.buf);
        return h;
    }

    hash_t hash(tos::span<const char> seq) const {
        return hash(tos::span<const uint8_t>{(const uint8_t*)seq.data(), seq.size()});
    }

    hash_t hash(const uint64_t& seq) const {
        return hash(tos::span<const uint8_t>{(const uint8_t*)&seq, sizeof seq});
    }
};

class signer : public tos::non_copy_movable {
public:
    explicit signer(tos::span<const char> key)
        : m_hmac{(const uint8_t*)key.data(), uint32_t(key.size())} {
    }

    void sign(tos::span<const uint8_t> msg, sign_t& sign) noexcept {
        m_hmac.update(msg.data(), msg.size());
        m_hmac.finalize(sign.signature()->GetMutablePointer(0));
        m_hmac.reset();
    }

    sign_t sign(tos::span<const uint8_t> msg) noexcept {
        sign_t signature;
        sign(msg, signature);
        return signature;
    }

private:
    ::emsha::HMAC m_hmac;
};

struct model {
    using signer_type = signer;
    using hasher_type = hasher;

    using sign_type = sign_t;
    using hash_type = hash_t;
};
} // namespace emsha
} // namespace caps