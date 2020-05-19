//
// Created by fatih on 5/18/18.
//

#include "doctest.h"

#include <tos/intrusive_list.hpp>

namespace tos {
namespace {
class foo : public tos::list_node<foo> {
public:
    int x;
    foo(int x)
        : x{x} {
    }
};

struct bar {
    int x;
    list_node<bar> list;
};

TEST_CASE("push_back") {
    intrusive_list<foo> foos;
    REQUIRE(foos.size() == 0);

    foo a{3};

    foos.push_back(a);

    REQUIRE(foos.size() == 1);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &a);

    foos.pop_front();
    REQUIRE(foos.size() == 0);
}

TEST_CASE("insert") {
    intrusive_list<foo> foos;
    REQUIRE(foos.size() == 0);

    foo a{3};

    foos.insert(foos.begin(), a);

    REQUIRE(foos.size() == 1);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &a);

    foo b{4};

    foos.insert(foos.begin(), b);

    REQUIRE(foos.size() == 2);
    REQUIRE(&foos.front() == &b);
    REQUIRE(&foos.back() == &a);
}

TEST_CASE("insert at end") {
    intrusive_list<foo> foos;
    REQUIRE(foos.size() == 0);

    foo a{3};

    foos.insert(foos.end(), a);

    REQUIRE(foos.size() == 1);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &a);
}

TEST_CASE("insert in middle") {
    intrusive_list<foo> foos;

    foo a{3};
    foo b{4};
    foo c{5};

    foos.push_back(a);
    foos.push_back(c);
    REQUIRE(foos.size() == 2);
    REQUIRE(&foos.front() == &a);
    REQUIRE(&foos.back() == &c);

    foos.insert(++foos.begin(), b);
    REQUIRE(&*(++foos.begin()) == &b);
}

TEST_CASE("pop back") {
    intrusive_list<foo> foos;

    foo a{3};
    foo b{4};
    foo c{5};

    foos.push_back(a);
    foos.push_back(b);
    foos.push_back(c);

    REQUIRE(&foos.back() == &c);
    foos.pop_back();
    REQUIRE(&foos.back() == &b);
    foos.pop_back();
    REQUIRE(&foos.back() == &a);
    foos.pop_back();
    REQUIRE(foos.empty());
}

TEST_CASE("erase") {
    intrusive_list<foo> foos;

    foo a{3};
    auto it = foos.push_back(a);
    foos.erase(it);

    REQUIRE(foos.empty());
}

TEST_CASE("Member lists work") {
    intrusive_list<bar, through_member<&bar::list>> list;
    bar b{3};
    list.push_back(b);
    REQUIRE_EQ(&b, &list.front());
}
} // namespace
} // namespace tos