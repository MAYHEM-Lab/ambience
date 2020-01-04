//
// Created by fatih on 12/30/19.
//

#pragma once

#include "crypto.hpp"

#include <caps/crypto/sha2_model.hpp>
#include <ti/drivers/crypto/CryptoCC32XX.h>
#include <tos/debug/panic.hpp>

namespace tos::cc32xx {
class sha2_hasher {
public:
private:
};
class hmac_sha2_signer {
public:
    explicit hmac_sha2_signer(const crypto&, tos::span<const uint8_t> secret) {
        m_handle = CryptoCC32XX_open(0, CryptoCC32XX_HMAC);
        if (!m_handle) {
            tos::debug::panic("can't open crypto");
        }
        CryptoCC32XX_HmacParams_init(&m_params);
        m_params.moreData = true;
    }

    template<class... MessageTs>
    caps::sha2::sign_t operator()(MessageTs&&... messages) const {
    }

    ~hmac_sha2_signer() {
        CryptoCC32XX_close(m_handle);
    }

private:
    void sign_one(tos::span<const uint8_t> buffer) {
        CryptoCC32XX_sign(m_handle,
                          CryptoCC32XX_HMAC_SHA256,
                          const_cast<uint8_t*>(buffer.data()),
                          buffer.size(),
                          m_digest.data(),
                          &m_params);
    }

    void sign_last(tos::span<const uint8_t> buffer) {
        sign_one(buffer);
    }

    std::array<uint8_t, 32> m_digest;
    CryptoCC32XX_HmacParams m_params;
    CryptoCC32XX_Handle m_handle;
};
}
