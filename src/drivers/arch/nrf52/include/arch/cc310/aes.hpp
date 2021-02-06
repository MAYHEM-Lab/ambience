#pragma once

#include "cryptocell.hpp"

#include <crys_aesccm.h>
#include <ssi_aes.h>
#include <tos/debug/assert.hpp>
#include <tos/span.hpp>

namespace tos::nrf52::cc310 {
class aes_ctr {
public:
    static expected<aes_ctr, cc_errors> make(span<const uint8_t> key,
                                             span<const uint8_t> iv) {
        if (iv.size() != SASI_AES_IV_SIZE_IN_BYTES) {
            return unexpected(cc_errors::bad_iv_length);
        }

        aes_ctr instance;

        auto res = SaSi_AesInit(&instance.m_enc_context,
                                SaSiAesEncryptMode_t::SASI_AES_ENCRYPT,
                                SaSiAesOperationMode_t::SASI_AES_MODE_CTR,
                                SaSiAesPaddingType_t::SASI_AES_PADDING_NONE);

        if (res != SASI_OK) {
            return unexpected(cc_errors::unknown_error);
        }

        res = SaSi_AesInit(&instance.m_dec_context,
                           SaSiAesEncryptMode_t::SASI_AES_DECRYPT,
                           SaSiAesOperationMode_t::SASI_AES_MODE_CTR,
                           SaSiAesPaddingType_t::SASI_AES_PADDING_NONE);

        if (res != SASI_OK) {
            return unexpected(cc_errors::unknown_error);
        }

        instance.m_key.pKey = const_cast<uint8_t*>(key.data());
        instance.m_key.keySize = key.size();

        res = SaSi_AesSetKey(&instance.m_enc_context,
                             SaSiAesKeyType_t::SASI_AES_USER_KEY,
                             &instance.m_key,
                             sizeof instance.m_key);

        if (res != SASI_OK) {
            return unexpected(cc_errors::bad_key);
        }

        res = SaSi_AesSetKey(&instance.m_dec_context,
                             SaSiAesKeyType_t::SASI_AES_USER_KEY,
                             &instance.m_key,
                             sizeof instance.m_key);

        if (res != SASI_OK) {
            return unexpected(cc_errors::bad_key);
        }

        res = SaSi_AesSetIv(&instance.m_enc_context, const_cast<uint8_t*>(iv.data()));

        if (res != SASI_OK) {
            return unexpected(cc_errors::bad_iv);
        }

        res = SaSi_AesSetIv(&instance.m_dec_context, const_cast<uint8_t*>(iv.data()));

        if (res != SASI_OK) {
            return unexpected(cc_errors::bad_iv);
        }

        return instance;
    }

    expected<void, cc_errors> encrypt_block(span<uint8_t> block) {
        AssertEq(SASI_AES_BLOCK_SIZE_IN_BYTES, block.size());
        auto ret =
            SaSi_AesBlock(&m_enc_context,
                          block.data(),
                          block.size(),
                          block.data());
        if (ret != SASI_OK) {
            return unexpected(cc_errors::unknown_error);
        }
        return {};
    }

    expected<void, cc_errors> decrypt_block(span<uint8_t> block) {
        AssertEq(SASI_AES_BLOCK_SIZE_IN_BYTES, block.size());
        auto ret =
            SaSi_AesBlock(&m_dec_context,
                          block.data(),
                          block.size(),
                          block.data());
        if (ret != SASI_OK) {
            return unexpected(cc_errors::unknown_error);
        }
        return {};
    }

private:
    SaSiAesUserContext_t m_enc_context;
    SaSiAesUserContext_t m_dec_context;
    SaSiAesUserKeyData_t m_key;
};
} // namespace tos::nrf52::cc310