#include <caplets_generated.hpp>
#include <timeout_generated.hpp>
#include <caplets_benchmark_generated.hpp>

#include <tos/caplets.hpp>
#include <tos/caplets/crypto.hpp>
#include <tos/debug/debug.hpp>
#include <tos/ubench/bench.hpp>

namespace caplets {
template<class CtxT>
bool verify(const TimeoutConstraint& cstr, const CtxT&) {
    return cstr.expires_at().ts() == 1234;
}
} // namespace caplets

namespace {
void BM_CapletsVerify(tos::bench::any_state& state) {
    std::array<uint8_t, 256> buf;
    lidl::message_builder builder(buf);

    capletsbench::Constraints ctr(caplets::TimeoutConstraint(caplets::Time(1234)));

    auto& frame = lidl::create<capletsbench::Frame>(
        builder,
        lidl::create_vector<capletsbench::Capabilities>(builder),
        lidl::create_vector<capletsbench::Constraints>(builder,
                                                       tos::monospan(ctr)));

    auto& tok = lidl::create<capletsbench::Token>(
        builder,
        caplets::Tag{{}},
        lidl::create_vector<capletsbench::Frame>(
            builder,
            frame, frame, frame, frame));

    uint8_t key[] = "iadasfasfasfadgasdfasfasfasfasfasfasfasgasdfasfasgadgasdasdasfasdf";
    tok.tag() = caplets::compute_token_tag<crypto>(tok, key);

    LOG(builder.size());

    for (auto _ : state) {
        bool sign = tok.tag() == caplets::compute_token_tag<crypto>(tok, key);
        auto deriv = caplets::validate_token(tok);

        caplets::common_validation_context<capletsbench::Token> ctx;
        ctx.token = &tok;

        auto constr = caplets::check_constraints(ctx);
        tos::debug::do_not_optimize(&sign);
        tos::debug::do_not_optimize(&deriv);
        tos::debug::do_not_optimize(&constr);
    }
}

BENCHMARK(BM_CapletsVerify);
} // namespace