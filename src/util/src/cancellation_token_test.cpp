#include <doctest.h>
#include <tos/cancellation_token.hpp>

namespace tos {
namespace {
TEST_CASE("Cancellation token destructor works") {
    {
        auto tok = cancellation_token::system().nest();

        REQUIRE_EQ(2, ad_hoc_list(tok).size());

        tok.cancel();

        REQUIRE_EQ(1, ad_hoc_list(tok).size());
    }
    {
        auto tok = cancellation_token::system().nest();
        auto tok2 = tok.nest();

        REQUIRE_EQ(3, ad_hoc_list(tok).size());

        tok.cancel();

        REQUIRE_EQ(1, ad_hoc_list(tok).size());
    }
    {
        auto tok = cancellation_token::system().nest();
        auto tok2 = tok.nest();
        auto tok3 = tok.nest();

        REQUIRE_EQ(4, ad_hoc_list(tok).size());
    }

    {
        auto tok = cancellation_token::system().nest();
        auto tok2 = tok.nest();
        auto tok3 = tok2.nest();

        REQUIRE_EQ(4, ad_hoc_list(tok).size());

        tok.cancel();

        REQUIRE_EQ(1, ad_hoc_list(tok).size());
    }

    REQUIRE_EQ(1, ad_hoc_list(cancellation_token::system()).size());
}

TEST_CASE("Cancellation token works") {
    auto tok = cancellation_token::system().nest();

    bool set = false;
    auto fn = [&] { set = true; };

    tok.set_cancel_callback(tos::function_ref<void()>(fn));

    tok.cancel();

    REQUIRE(set);
}

TEST_CASE("Cancellation token nesting works") {
    auto parent_tok = cancellation_token::system().nest();
    auto tok = parent_tok.nest();

    bool set = false;
    auto fn = [&] { set = true; };

    tok.set_cancel_callback(tos::function_ref<void()>(fn));

    parent_tok.cancel();

    REQUIRE(set);
}

TEST_CASE("Cancellation token tree works") {
    auto parent_tok = cancellation_token::system().nest();
    auto tok = parent_tok.nest();
    auto tok2 = parent_tok.nest();

    bool set = false;
    bool set2 = false;
    auto fn = [&] { set = true; };
    auto fn2 = [&] { set2 = true; };

    tok.set_cancel_callback(tos::function_ref<void()>(fn));
    tok2.set_cancel_callback(tos::function_ref<void()>(fn2));

    parent_tok.cancel();

    REQUIRE(set);
    REQUIRE(set2);
}

TEST_CASE("Cancellation token tree works") {
    auto parent_tok = cancellation_token::system().nest();
    auto tok = parent_tok.nest();
    auto tok2 = parent_tok.nest();

    bool set = false;
    bool set2 = false;
    auto fn = [&] { set = true; };
    auto fn2 = [&] { set2 = true; };

    tok.set_cancel_callback(tos::function_ref<void()>(fn));
    tok2.set_cancel_callback(tos::function_ref<void()>(fn2));

    tok.cancel();

    REQUIRE(set);
    REQUIRE_FALSE(set2);
}
} // namespace
} // namespace tos