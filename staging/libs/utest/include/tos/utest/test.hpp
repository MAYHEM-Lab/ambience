#pragma once

#include <string_view>
#include <tos/debug/assert.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/debug/log.hpp>

namespace tos::utest {
struct test_case_base : list_node<test_case_base> {
    virtual bool run();
    virtual ~test_case_base() = default;

    std::string_view name;
};

inline intrusive_list<test_case_base> all_tests;

template<class TestCase>
struct test_case : test_case_base {
    test_case(std::string_view name) {
        this->name = name;
        all_tests.push_back(*this);
    }

protected:
    void test(std::string_view name, void (TestCase::*fn)()) {
        LOG("Running", name);
        (static_cast<TestCase*>(this)->*fn)();
    }
};

template<class Left, class Right>
void require_eq_impl(const Left& left, const Right& right) {
    if (left != right) {
        LOG_ERROR(left, "!=", right);
    }
}
} // namespace tos::utest

#define REQUIRE(x) Assert(x)
#define REQUIRE_EQ(x, y) (::tos::utest::require_eq_impl(x, y), REQUIRE(x == y))
