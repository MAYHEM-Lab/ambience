#pragma once

#include <crys_aesccm.h>
#include <crys_error.h>
#include <crys_hash.h>
#include <crys_hmac.h>
#include <cstdint>
#include <etl/vector.h>
#include <tos/debug/panic.hpp>
#include <tos/span.hpp>
#include <tos/thread.hpp>

namespace tos::nrf52::cc310 {
class hmac_sha2_signer {
public:
    explicit hmac_sha2_signer(tos::span<const uint8_t> key)
        : m_key(key.begin(), key.end()) {
        auto res = CRYS_HMAC_Init(&m_ctx,
                                  CRYS_HASH_SHA256_mode,
                                  const_cast<uint8_t*>(m_key.data()),
                                  m_key.size());
        if (res != CRYS_OK) {
            tos::debug::panic("Can't initialize hmac!");
        }
    }

    template<class... MessageTs>
    std::array<uint8_t, 32> operator()(MessageTs&&... messages) const {
        (void)((sign_one(std::forward<MessageTs>(messages)), ...));

        if (!m_buf.empty()) {
            definitely_sign_one({m_buf.data(), m_buf.size()});
        }

        alignas(uint32_t) CRYS_HASH_Result_t result;
        CRYS_HMAC_Finish(&m_ctx, result);

        auto res = CRYS_HMAC_Init(&m_ctx,
                                  CRYS_HASH_SHA256_mode,
                                  const_cast<uint8_t*>(m_key.data()),
                                  m_key.size());
        if (res != CRYS_OK) {
            tos::debug::panic("Can't initialize hmac!");
        }
        m_buf.clear();

        std::array<uint8_t, 32> sign;
        memcpy(sign.data(), result, sign.size());
        return sign;
    }

private:
    void definitely_sign_one(tos::span<const uint8_t> message) const {
        auto res = CRYS_HMAC_Update(
            &m_ctx, const_cast<uint8_t*>(message.data()), message.size());
        if (res != CRYS_OK) {
            tos::debug::panic("cc310 hmac update failed!");
        }
    }

    void sign_one(tos::span<const uint8_t> msg) const {
        while (msg.size() >= 64) {
            definitely_sign_one(msg.slice(0, 64));
            msg = msg.slice(64);
        }
        leftover(msg);
    }

    void leftover(tos::span<const uint8_t> msg) const {
        auto merged_size = std::min<size_t>(64, msg.size() + m_buf.size());
        auto copy_size = merged_size - m_buf.size();
        m_buf.insert(m_buf.end(), msg.begin(), msg.begin() + copy_size);
        if (m_buf.size() == 64) {
            definitely_sign_one({m_buf.data(), m_buf.size()});
            m_buf.clear();
        }
        m_buf.insert(m_buf.end(), msg.begin() + copy_size, msg.end());
    }
    mutable etl::vector<uint8_t, 64> m_key;
    mutable etl::vector<uint8_t, 64> m_buf;
    mutable CRYS_HMACUserContext_t m_ctx;
};

class sha2_hasher {
public:
    sha2_hasher() {
        auto res = CRYS_HASH_Init(&m_ctx, CRYS_HASH_SHA256_mode);
        if (res != CRYS_OK) {
            tos::debug::panic("Can't initialize hash!");
        }
    }

    template<class... MessageTs>
    std::array<uint8_t, 32> operator()(MessageTs&&... messages) const {
        (void)((hash_one(std::forward<MessageTs>(messages)), ...));

        if (!m_buf.empty()) {
            definitely_hash_one({m_buf.data(), m_buf.size()});
        }

        alignas(uint32_t) CRYS_HASH_Result_t result;
        CRYS_HASH_Finish(&m_ctx, result);

        std::array<uint8_t, 32> h;
        memcpy(h.data(), result, h.size());

        auto res = CRYS_HASH_Init(&m_ctx, CRYS_HASH_SHA256_mode);
        if (res != CRYS_OK) {
            tos::debug::panic("Can't initialize hash!");
        }
        m_buf.clear();

        return h;
    }

private:
    void definitely_hash_one(tos::span<const uint8_t> msg) const {
        auto res = CRYS_HASH_Update(&m_ctx, const_cast<uint8_t*>(msg.data()), msg.size());
        if (res != CRYS_OK) {
            // error
            tos::debug::panic("Can't update hash!");
        }
    }

    void hash_one(tos::span<const uint8_t> msg) const {
        while (msg.size() >= 64) {
            definitely_hash_one(msg.slice(0, 64));
            msg = msg.slice(64);
        }
        leftover(msg);
    }

    void leftover(tos::span<const uint8_t> msg) const {
        auto merged_size = std::min<size_t>(64, msg.size() + m_buf.size());
        auto copy_size = merged_size - m_buf.size();
        m_buf.insert(m_buf.end(), msg.begin(), msg.begin() + copy_size);
        if (m_buf.size() == 64) {
            definitely_hash_one({m_buf.data(), m_buf.size()});
            m_buf.clear();
        }
        m_buf.insert(m_buf.end(), msg.begin() + copy_size, msg.end());
    }

    mutable etl::vector<uint8_t, 64> m_buf;
    mutable CRYS_HASHUserContext_t m_ctx;
};
} // namespace tos::nrf52::cc310