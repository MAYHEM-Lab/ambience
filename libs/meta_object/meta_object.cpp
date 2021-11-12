#include "tos/meta_object/dictionary.hpp"
#include <tos/meta_object.hpp>

namespace tos::meta_object {
static_assert(is_same<fixed_string("foo"), fixed_string("foo")>::value);

static_assert(meta::has_key<"foo", dictionary<kv<"foo", int>>>::value);

constexpr auto foo() {
    constexpr dictionary<kv<"foo", int>> d{
        key<"foo"> = 42
    };
    return d;
}

constexpr auto bar() {
    constexpr dictionary<kv<"foo", int>, kv<"bar", float>> d{
        key<"foo"> = 42,
        key<"bar"> = 3.14f
    };
    return d;
}

constexpr auto test_dict() {
    dictionary<kv<"foo", int>, kv<"yolo", float>, kv<"str", std::string_view>> d{
        key<"yolo"> = 3.14f
    };

    get<"foo">(d) = 42;
    get<"str">(d) = "hello world";
    d[key<"foo">] = 42;
    return get<"foo">(d) == 42 && get<"str">(d).size() == 11 && get<"yolo">(d) != 0;
}

constexpr auto get_int(const dictionary<kv<"foo", int>>& arg) {
    return arg[key<"foo">];
}

static_assert(test_dict());
static_assert(get_int(foo()) == 42);
static_assert(get_int(bar()) == 42);
} // namespace tos::meta_object