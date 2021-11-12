#include <doctest.h>
#include <tos/meta_object/dictionary.hpp>
#include <tos/meta_object/print.hpp>
#include <tos/mem_stream.hpp>

namespace tos::meta_object {
namespace {
TEST_CASE("dictionary printing works") {
    uint8_t buf[512];
    tos::omemory_stream str(buf);
    dictionary<kv<"foo", int>> d {
        key<"foo"> = 42
    };
    print(str, d);
    auto x = std::string_view((const char*)str.get().data(), str.get().size());
    REQUIRE_EQ("{\"foo\" : 42}", x);
}
} // namespace
} // namespace tos::meta_object