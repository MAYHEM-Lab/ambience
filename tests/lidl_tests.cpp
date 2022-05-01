#include "fmt/format.h"
#include "lidlrt/builder.hpp"
#include "lidlrt/vector.hpp"
#include <doctest.h>
#include <lidlrt/format.hpp>
#include <networking_generated.hpp>

namespace {
TEST_CASE("Formatting works") {
    std::vector<uint8_t> buffer(1024);
    lidl::message_builder mb(buffer);

    auto& obj = lidl::create<tos::services::recvfrom_res>(
        mb,
        lidl::create_vector_sized<uint8_t>(mb, 128),
        tos::services::udpv4_endpoint{tos::services::ipv4_addr{lidl::array<uint8_t, 4>{0xa, 0xb, 0xc, 0xd}},
                                      tos::services::ip_port{99}});
    
    auto arr_str = fmt::to_string(lidl::array<uint8_t, 4>{0xa, 0xb, 0xc, 0xd});
    auto vec_str = fmt::to_string(lidl::create_vector_sized<uint8_t>(mb, 128));
    auto obj_str = fmt::to_string(obj);

    fmt::print("{}\n", arr_str);
    fmt::print("{}\n", vec_str);
    fmt::print("{}\n", obj_str);
}
} // namespace