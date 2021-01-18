#include <emsha/hmac.hh>
#include <emsha/sha256.hh>
#include <tos/debug/debug.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>
#include <uECC.h>

namespace {
namespace caps {
namespace emsha {
struct hasher {
public:
    template<class... MessageTs>
    std::array<uint8_t, 32> operator()(MessageTs&&... messages) const {
        auto hash_one = [&](tos::span<const uint8_t> msg) {
          m_hash.update(msg.data(), msg.size());
          return 0;
        };
        (void)(hash_one(std::forward<MessageTs>(messages)) + ...);

        std::array<uint8_t, 32> h;
        m_hash.finalize(h.data());
        m_hash.reset();
        return h;
    }

private:
    mutable ::emsha::SHA256 m_hash;
};

class signer {
public:
    explicit signer(tos::span<const uint8_t> key)
        : m_hmac{key.data(), uint32_t(key.size())} {
    }

    template<class... MessageTs>
    std::array<uint8_t, 32> operator()(MessageTs&&... messages) const noexcept {
        auto sign_one = [&](tos::span<const uint8_t> msg) {
          m_hmac.update(msg.data(), msg.size());
          return 0;
        };
        (void)(sign_one(std::forward<MessageTs>(messages)) + ...);

        std::array<uint8_t, 32> signature;
        m_hmac.finalize(signature.data());
        m_hmac.reset();
        return signature;
    }

private:
    mutable ::emsha::HMAC m_hmac;
};
} // namespace emsha
} // namespace caps

void BM_EMSHA_HMAC(tos::bench::any_state& state) {
    char buf[] = "hello world";

    caps::emsha::signer sign(tos::raw_cast<const uint8_t>(tos::span("sekret")));

    for (auto _ : state) {
        auto sign_val = sign(tos::raw_cast<const uint8_t>(tos::span(buf)));
        tos::debug::do_not_optimize(&sign_val);
    }
}

void BM_UECC_ECDSA_Sign(tos::bench::any_state& state) {
    auto curve = uECC_secp256r1();

    uECC_set_rng([](uint8_t* dest, unsigned size) {
      return 1;
    });


    uint8_t private1[32];

    uint8_t public1[64];

    auto res = uECC_make_key(public1, private1, curve);

    caps::emsha::hasher hasher;
    char buf[] = "hello world";

    auto hash = hasher(tos::raw_cast<uint8_t>(tos::span(buf)));

    for (auto _ : state) {
        uint8_t signature[64];
        auto sign_res =
            uECC_sign(private1, hash.data(), hash.size(), signature, curve);
        tos::debug::do_not_optimize(signature);
    }
    LOG("Bye");
}

BENCHMARK(BM_EMSHA_HMAC);
BENCHMARK(BM_UECC_ECDSA_Sign);
}