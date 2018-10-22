//
// Created by fatih on 10/19/18.
//

#pragma once

#include <emsha/hmac.hh>
#include <tos/span.hpp>

namespace caps
{
    class signer {
    public:
        explicit signer(tos::span<const char> key)
            : m_hmac{(const uint8_t *)key.data(), key.size()} {}

        signer(const signer &) = delete;

        void sign(tos::span<const uint8_t> msg, tos::span<uint8_t> out) noexcept
        {
            m_hmac.update(msg.data(), msg.size());
            m_hmac.finalize(out.data());
            m_hmac.reset();
        }

    private:
        emsha::HMAC m_hmac;
    };
}