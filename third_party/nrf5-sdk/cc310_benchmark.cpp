#include <arch/cc310/aes.hpp>
#include <arch/cc310/cryptocell.hpp>
#include <arch/cc310/rng.hpp>
#include <arch/cc310/sha2.hpp>
#include <crys_ec_mont_api.h>
#include <tos/debug/debug.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>

namespace bench = tos::bench;
namespace {
void BM_CC310_HMAC(bench::any_state& state) {
    auto cc = tos::nrf52::cc310::open_cryptocell();
    if (!cc) {
        tos::this_thread::block_forever();
    }

    char buf[] = "hello world";
    tos::nrf52::cc310::hmac_sha2_signer sign(
        tos::raw_cast<const uint8_t>(tos::span("sekret")));

    for (auto _ : state) {
        auto sign_val = sign(tos::raw_cast<const uint8_t>(tos::span(buf)));
        tos::debug::do_not_optimize(sign_val);
    }
}

uint8_t data[64] = "foobar";

void BM_CC310_AES_CTR(bench::any_state& state) {
    auto cc = tos::nrf52::cc310::open_cryptocell();
    if (!cc) {
        tos::this_thread::block_forever();
    }

    constexpr uint8_t key[16] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
    constexpr uint8_t iv[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    LOG("Initializing");
    auto encrypt = force_get(tos::nrf52::cc310::aes_ctr::make(key, iv));
    LOG("Initialized");
    for (auto _ : state) {
        for (int i = 0; i < 4; ++i) {
            auto res = encrypt.encrypt_block(tos::span(data).slice(i * 16, 16));
            tos::debug::do_not_optimize(data);
        }
    }
}

void BM_CC310_ECDHE(bench::any_state& state) {
    auto cc = tos::nrf52::cc310::open_cryptocell();
    if (!cc) {
        tos::this_thread::block_forever();
    }

    CRYS_ECMONT_TempBuff_t tmp{};

    uint8_t pub[40];
    size_t pubsz = sizeof pub;

    uint8_t pub2[40];
    size_t pub2sz = sizeof pub2;

    uint8_t priva[32];
    size_t privasz = sizeof priva;

    uint8_t privb[32] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                         17, 18, 19, 20, 21, 22, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32};

    auto gen_res = CRYS_ECMONT_SeedKeyPair(pub, &pubsz, priva, &privasz, privb, sizeof privb, &tmp);
    LOG(gen_res, tos::span(pub).slice(0, pubsz), pubsz);
    auto res = CRYS_ECMONT_ScalarmultBase(pub2, &pub2sz, priva, sizeof priva, &tmp);
    LOG(res, tos::span(pub2).slice(0, pub2sz));

    for (auto _ : state) {
        auto res = CRYS_ECMONT_ScalarmultBase(pub2, &pub2sz, priva, sizeof priva, &tmp);

        uint8_t dhres[40];
        size_t ressz = sizeof dhres;
        auto res2 =
            CRYS_ECMONT_Scalarmult(dhres, &ressz, privb, sizeof privb, pub, pubsz, &tmp);
        tos::debug::do_not_optimize(dhres);
    }
}

BENCHMARK(BM_CC310_HMAC);
BENCHMARK(BM_CC310_ECDHE);
BENCHMARK(BM_CC310_AES_CTR);
} // namespace