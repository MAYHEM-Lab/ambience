#pragma once

#include <tos/caplets/tag.hpp>
#include <array>
#include <emsha/hmac.hh>
#include <tos/span.hpp>

struct crypto {
    crypto(const std::array<uint8_t, 32>& key) : m_hmac(key.data(), key.size()) {}

    void append(tos::span<const uint8_t> buf) const{
        m_hmac.update(buf.data(), buf.size());
    }

    caplets::Tag finish() {
        caplets::Tag res{{}};
        m_hmac.finalize(res.tag().data());
        return res;
    }

    mutable emsha::HMAC m_hmac;
};