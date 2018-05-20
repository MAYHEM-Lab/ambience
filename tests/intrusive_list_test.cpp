//
// Created by fatih on 5/18/18.
//

#include "catch.hpp"
#include <tos/intrusive_list.hpp>

class foo : public tos::list_node<foo>
{
public:
    int x;
    foo(int x) : x{x} {}
};

TEST_CASE("push_back", "[intrusive_list]")
{
    using namespace tos;
    intrusive_list<foo> foos;
    REQUIRE(foos.size() == 0);

    foo a { 3 };

    foos.push_back(a);

    REQUIRE(foos.size() == 1);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &a);

    foos.pop_front();
    REQUIRE(foos.size() == 0);
}

TEST_CASE("insert", "[intrusive_list]")
{
    using namespace tos;
    intrusive_list<foo> foos;
    REQUIRE(foos.size() == 0);

    foo a { 3 };

    foos.insert(foos.begin(), a);

    REQUIRE(foos.size() == 1);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &a);
}

TEST_CASE("insert at end", "[intrusive_list]")
{
    using namespace tos;
    intrusive_list<foo> foos;
    REQUIRE(foos.size() == 0);

    foo a { 3 };

    foos.insert(foos.end(), a);

    REQUIRE(foos.size() == 1);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &a);
}