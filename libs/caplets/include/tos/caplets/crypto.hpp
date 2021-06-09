#pragma once

#include <array>
#include <sha256.h>
#include <tos/caplets/tag.hpp>
#include <tos/span.hpp>

struct crypto {
    crypto(tos::span<const uint8_t> key) {
        reset(key);
    }

    void reset(tos::span<const uint8_t> key) {
        HMAC_SHA256_Init(&m_buf, key.data(), key.size());
    }

    void append(tos::span<const uint8_t> buf) const {
        HMAC_SHA256_Update(&m_buf, buf.data(), buf.size());
    }

    caplets::Tag finish() {
        caplets::Tag res{{}};
        HMAC_SHA256_Final(res.tag().data(), &m_buf);
        return res;
    }

    mutable HMAC_SHA256_CTX m_buf;
};