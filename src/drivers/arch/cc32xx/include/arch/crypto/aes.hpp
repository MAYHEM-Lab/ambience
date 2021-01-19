#pragma once

#include <tos/span.hpp>
#include <ti/drivers/crypto/CryptoCC32XX.h>
#include <tos/expected.hpp>

namespace tos::cc32xx {
enum class crypto_errors {
    unknown_error
};

class aes_cbc : tos::non_copy_movable {
public:
    aes_cbc(span<const uint8_t> key, span<const uint8_t> iv);

    expected<void, crypto_errors> encrypt_block(span<uint8_t> block);
    expected<void, crypto_errors> decrypt_block(span<uint8_t> block);

    ~aes_cbc();
private:

    CryptoCC32XX_Handle m_handle;
    CryptoCC32XX_EncryptParams m_params;
};

class aes_ctr : tos::non_copy_movable {
public:
    aes_ctr(span<const uint8_t> key, span<const uint8_t> iv);

    expected<void, crypto_errors> encrypt_block(span<uint8_t> block);
    expected<void, crypto_errors> decrypt_block(span<uint8_t> block);

    ~aes_ctr();
private:

    CryptoCC32XX_Handle m_handle;
    CryptoCC32XX_EncryptParams m_params;
};
}