#include <arch/crypto/crypto.hpp>
#include <arch/crypto/aes.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>
#include <tos/debug/debug.hpp>

namespace tos::cc32xx {
namespace {
uint8_t data[64] = "foobar";

void BM_HW_AES(bench::any_state& state) {
    tos::cc32xx::crypto c;
    constexpr uint8_t key[16] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
    uint8_t iv[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    tos::cc32xx::aes_ctr encrypt(key, iv);
    for (auto _ : state) {
        for (int i = 0; i < 4; ++i) {
            auto res = encrypt.encrypt_block(tos::span(data).slice(i * 16, 16));
            tos::debug::do_not_optimize(&res);
        }
    }
}

BENCHMARK(BM_HW_AES);
}
}