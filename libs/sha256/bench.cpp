#include "sha256.h"
#include <tos/ubench/state.hpp>
#include <tos/ubench/bench.hpp>

namespace {
int macaroon_hmac(const unsigned char* _key,
                  size_t _key_sz,
                  const unsigned char* text,
                  size_t text_sz,
                  unsigned char* hash) {
    constexpr auto MACAROON_HASH_BYTES = 32;
    unsigned char key[MACAROON_HASH_BYTES];
    explicit_bzero(key, MACAROON_HASH_BYTES);
    memmove(key, _key, _key_sz < sizeof(key) ? _key_sz : sizeof(key));
    HMAC_SHA256_Buf(key, MACAROON_HASH_BYTES, text, text_sz, hash);
    return 0;
}

void BM_HMAC_SHA256(tos::bench::any_state& state) {
    auto key = reinterpret_cast<const uint8_t *>(
        "iadasfasfasfadgasdfasfasfasfafab");
    for (auto _ : state) {
        uint8_t out[32];
        macaroon_hmac(key, 32, reinterpret_cast<const uint8_t*>("hello world"), 12, out);
    }
}

BENCHMARK(BM_HMAC_SHA256);
} // namespace