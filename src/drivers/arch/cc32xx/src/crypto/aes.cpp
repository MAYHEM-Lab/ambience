#include <arch/crypto/aes.hpp>
#include <tos/debug/panic.hpp>
#include <tos/debug/log.hpp>

namespace tos::cc32xx {
aes_cbc::aes_cbc(span<const uint8_t> key, span<const uint8_t> iv) {
    m_handle = CryptoCC32XX_open(0, CryptoCC32XX_AES);
    if (!m_handle) {
        tos::debug::panic("can't open crypto");
    }
    switch (key.size()) {
    case 128 / 8:
        LOG_TRACE("Using AES-128");
        m_params.aes.keySize = CryptoCC32XX_AesKeySize::CryptoCC32XX_AES_KEY_SIZE_128BIT;
        break;
    case 192 / 8:
        LOG_TRACE("Using AES-192");
        m_params.aes.keySize = CryptoCC32XX_AesKeySize::CryptoCC32XX_AES_KEY_SIZE_192BIT;
        break;
    case 256 / 8:
        LOG_TRACE("Using AES-256");
        m_params.aes.keySize = CryptoCC32XX_AesKeySize::CryptoCC32XX_AES_KEY_SIZE_256BIT;
        break;
    default:
        tos::debug::panic("Bad AES key size!");
    }
    m_params.aes.pKey = key.data();
    m_params.aes.pIV = const_cast<uint8_t*>(iv.data());
}

expected<void, crypto_errors> aes_cbc::encrypt_block(span<uint8_t> block) {
    size_t out_len;
    auto res = CryptoCC32XX_encrypt(m_handle,
                                    CryptoCC32XX_AES_CBC,
                                    block.data(),
                                    block.size(),
                                    block.data(),
                                    &out_len,
                                    &m_params);
    if (res != CryptoCC32XX_STATUS_SUCCESS) {
        return unexpected(crypto_errors::unknown_error);
    }
    return {};
}

expected<void, crypto_errors> aes_cbc::decrypt_block(span<uint8_t> block) {
    size_t out_len;
    auto res = CryptoCC32XX_decrypt(m_handle,
                                    CryptoCC32XX_AES_CBC,
                                    block.data(),
                                    block.size(),
                                    block.data(),
                                    &out_len,
                                    &m_params);
    if (res != CryptoCC32XX_STATUS_SUCCESS) {
        return unexpected(crypto_errors::unknown_error);
    }
    return {};
}

aes_cbc::~aes_cbc() {
    CryptoCC32XX_close(m_handle);
}

aes_ctr::aes_ctr(span<const uint8_t> key, span<const uint8_t> iv) {
    m_handle = CryptoCC32XX_open(0, CryptoCC32XX_AES);
    if (!m_handle) {
        tos::debug::panic("can't open crypto");
    }
    switch (key.size()) {
    case 128 / 8:
        LOG_TRACE("Using AES-128");
        m_params.aes.keySize = CryptoCC32XX_AesKeySize::CryptoCC32XX_AES_KEY_SIZE_128BIT;
        break;
    case 192 / 8:
        LOG_TRACE("Using AES-192");
        m_params.aes.keySize = CryptoCC32XX_AesKeySize::CryptoCC32XX_AES_KEY_SIZE_192BIT;
        break;
    case 256 / 8:
        LOG_TRACE("Using AES-256");
        m_params.aes.keySize = CryptoCC32XX_AesKeySize::CryptoCC32XX_AES_KEY_SIZE_256BIT;
        break;
    default:
        tos::debug::panic("Bad AES key size!");
    }
    m_params.aes.pKey = key.data();
    m_params.aes.pIV = const_cast<uint8_t*>(iv.data());
}

expected<void, crypto_errors> aes_ctr::encrypt_block(span<uint8_t> block) {
    size_t out_len;
    auto res = CryptoCC32XX_encrypt(m_handle,
                                    CryptoCC32XX_AES_CTR,
                                    block.data(),
                                    block.size(),
                                    block.data(),
                                    &out_len,
                                    &m_params);
    if (res != CryptoCC32XX_STATUS_SUCCESS) {
        return unexpected(crypto_errors::unknown_error);
    }
    return {};
}

expected<void, crypto_errors> aes_ctr::decrypt_block(span<uint8_t> block) {
    size_t out_len;
    auto res = CryptoCC32XX_decrypt(m_handle,
                                    CryptoCC32XX_AES_CTR,
                                    block.data(),
                                    block.size(),
                                    block.data(),
                                    &out_len,
                                    &m_params);
    if (res != CryptoCC32XX_STATUS_SUCCESS) {
        return unexpected(crypto_errors::unknown_error);
    }
    return {};
}

aes_ctr::~aes_ctr() {
    CryptoCC32XX_close(m_handle);
}
} // namespace tos::cc32xx