//
// Created by fatih on 12/30/19.
//

#pragma once

#include "crypto.hpp"

#include <caps/crypto/sha2_model.hpp>
#include <ti/drivers/crypto/CryptoCC32XX.h>
#include <tos/debug/panic.hpp>
#include <tos/debug/log.hpp>

namespace tos::cc32xx {
class hmac_crypto {
public:
    explicit hmac_crypto(const crypto&) {
        m_handle = CryptoCC32XX_open(0, CryptoCC32XX_HMAC);
        if (!m_handle) {
            tos::debug::panic("can't open crypto");
        }
    }

    ~hmac_crypto() {
        CryptoCC32XX_close(m_handle);
    }

    CryptoCC32XX_Handle native_handle() {
        return m_handle;
    }

    static hmac_crypto& instance(const crypto& crypto) {
        static hmac_crypto c(crypto);
        return c;
    }

private:
    CryptoCC32XX_Handle m_handle;
};

class sha2_hasher {
public:
    explicit sha2_hasher(const crypto& c) {
        m_handle = hmac_crypto::instance(c).native_handle();
    }

    template<class... MessageTs>
    caps::sha2::hash_t operator()(MessageTs&&... messages) const {
        CryptoCC32XX_HmacParams_init(&m_params);
        m_tmp = tos::span<const uint8_t>{nullptr};

        (proc_one(messages), ...);

        if (!m_tmp.empty()) {
            proc_one_internal(m_tmp, false);
        }
        else {
            // All the messages were empty, the digest is garbage.
        }

        caps::sha2::hash_t sign;
        std::memcpy(
            sign.hash.data(), m_digest.data(), m_digest.size());
        return sign;
    }

private:
    void proc_one(tos::span<const uint8_t> buffer) const {
        if (buffer.empty()) {
            return;
        }

        if (m_tmp.empty()) {
            m_tmp = buffer;
            return;
        }

        proc_one_internal(m_tmp, true);
        m_tmp = buffer;
    }

    void proc_one_internal(tos::span<const uint8_t> buffer, bool more) const {
        DEBUG("Hashing", buffer);

        if (buffer.empty()) return;
        m_params.moreData = more;
        CryptoCC32XX_sign(m_handle,
                          CryptoCC32XX_HMAC_SHA256,
                          const_cast<uint8_t*>(buffer.data()),
                          buffer.size(),
                          m_digest.data(),
                          &m_params);
    }

    mutable std::array<uint8_t, 32> m_digest;
    mutable CryptoCC32XX_HmacParams m_params;
    CryptoCC32XX_Handle m_handle;
    mutable tos::span<const uint8_t> m_tmp{nullptr};
};

class hmac_sha2_signer {
public:
    hmac_sha2_signer(const crypto& c, tos::span<const uint8_t> secret) {
        m_handle = hmac_crypto::instance(c).native_handle();
        WARN_IF(secret.size() > 64)("HMAC SHA2 Secret too long!");
        std::copy_n(
            secret.begin(),
            std::min(secret.size(), m_key.size()),
            m_key.begin());
    }

    template<class... MessageTs>
    caps::sha2::sign_t operator()(MessageTs&&... messages) const {
        CryptoCC32XX_HmacParams_init(&m_params);
        m_params.pKey = const_cast<uint8_t*>(m_key.data());
        m_tmp = tos::span<const uint8_t>{nullptr};

        (proc_one(messages), ...);

        if (!m_tmp.empty()) {
            proc_one_internal(m_tmp, false);
        }
        else {
            // All the messages were empty, the digest is garbage.
        }

        caps::sha2::sign_t sign;
        std::memcpy(
            sign.signature()->GetMutablePointer(0), m_digest.data(), m_digest.size());
        return sign;
    }

private:
    void proc_one(tos::span<const uint8_t> buffer) const {
        if (buffer.empty()) {
            return;
        }

        if (m_tmp.empty()) {
            m_tmp = buffer;
            return;
        }

        proc_one_internal(m_tmp, true);
        m_tmp = buffer;
    }

    void proc_one_internal(tos::span<const uint8_t> buffer, bool more) const {
        DEBUG("Signing", buffer);
        m_params.moreData = more;
        auto res = CryptoCC32XX_sign(m_handle,
                          CryptoCC32XX_HMAC_SHA256,
                          const_cast<uint8_t*>(buffer.data()),
                          buffer.size(),
                          m_digest.data(),
                          &m_params);
        ERROR_IF(res != CryptoCC32XX_STATUS_SUCCESS)("Sign did not return succcess");
    }

    mutable std::array<uint8_t, 32> m_digest;
    // The SDK assumes the keys are always 512-bits (i.e. 64 bytes). We'll copy the key
    // into this array.
    std::array<uint8_t, 64> m_key{};
    mutable CryptoCC32XX_HmacParams m_params;
    CryptoCC32XX_Handle m_handle;
    mutable tos::span<const uint8_t> m_tmp{nullptr};
};
} // namespace tos::cc32xx
