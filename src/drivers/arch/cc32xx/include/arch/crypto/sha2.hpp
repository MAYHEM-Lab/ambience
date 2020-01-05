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
class sha2_hasher {
public:
    explicit sha2_hasher(const crypto&) {
        m_handle = CryptoCC32XX_open(0, CryptoCC32XX_HMAC);
        if (!m_handle) {
            tos::debug::panic("can't open crypto");
        }
    }

    template<class... MessageTs, class LastMessageT>
    caps::sha2::hash_t operator()(MessageTs&&... messages, LastMessageT&& last) const {
        CryptoCC32XX_HmacParams_init(&m_params);
        if constexpr (sizeof...(messages) > 0) {
            m_params.moreData = true;
            (proc_one(messages), ...);
            m_params.moreData = false;
        }
        return proc_last(last);
    }

    ~sha2_hasher() {
        CryptoCC32XX_close(m_handle);
    }

private:
    void proc_one(tos::span<const uint8_t> buffer) const {
        CryptoCC32XX_sign(m_handle,
                          CryptoCC32XX_HMAC_SHA256,
                          const_cast<uint8_t*>(buffer.data()),
                          buffer.size(),
                          m_digest.data(),
                          &m_params);
    }

    caps::sha2::hash_t proc_last(tos::span<const uint8_t> buffer) const {
        proc_one(buffer);
        caps::sha2::hash_t sign;
        std::memcpy(
            sign.hash.data(), m_digest.data(), m_digest.size());
        return sign;
    }

    mutable std::array<uint8_t, 32> m_digest;
    mutable CryptoCC32XX_HmacParams m_params;
    CryptoCC32XX_Handle m_handle;
};

class hmac_sha2_signer {
public:
    hmac_sha2_signer(const crypto&, tos::span<const uint8_t> secret) {
        m_handle = CryptoCC32XX_open(0, CryptoCC32XX_HMAC);
        if (!m_handle) {
            tos::debug::panic("can't open crypto");
        }
        WARN_IF(secret.size() > 64)("HMAC SHA2 Secret too long!");
        std::copy_n(
            secret.begin(),
            std::min(secret.size(), m_key.size()),
            m_key.begin());
    }

    template<class... MessageTs, class LastMessageT>
    caps::sha2::sign_t operator()(MessageTs&&... messages, LastMessageT&& last) const {
        CryptoCC32XX_HmacParams_init(&m_params);
        m_params.pKey = const_cast<uint8_t*>(m_key.data());

        if constexpr (sizeof...(messages) > 0) {
            m_params.moreData = true;
            (proc_one(messages), ...);
            m_params.moreData = false;
        }
        return proc_last(last);
    }

    ~hmac_sha2_signer() {
        CryptoCC32XX_close(m_handle);
    }

private:
    void proc_one(tos::span<const uint8_t> buffer) const {
        auto res = CryptoCC32XX_sign(m_handle,
                          CryptoCC32XX_HMAC_SHA256,
                          const_cast<uint8_t*>(buffer.data()),
                          buffer.size(),
                          m_digest.data(),
                          &m_params);
        ERROR_IF(res != CryptoCC32XX_STATUS_SUCCESS)("Sign did not return succcess");
    }

    caps::sha2::sign_t proc_last(tos::span<const uint8_t> buffer) const {
        proc_one(buffer);
        caps::sha2::sign_t sign;
        std::memcpy(
            sign.signature()->GetMutablePointer(0), m_digest.data(), m_digest.size());
        return sign;
    }

    mutable std::array<uint8_t, 32> m_digest;
    // The SDK assumes the keys are always 512-bits (i.e. 64 bytes). We'll copy the key
    // into this array.
    std::array<uint8_t, 64> m_key{};
    mutable CryptoCC32XX_HmacParams m_params;
    CryptoCC32XX_Handle m_handle;
};
} // namespace tos::cc32xx
