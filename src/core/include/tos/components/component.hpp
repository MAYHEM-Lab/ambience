#pragma once

#include <cstdint>

namespace tos {
using component_id_t = uint32_t;

template<typename T>
constexpr auto component_id() noexcept -> component_id_t {
    return T::id;
}

struct component {};

template<component_id_t ID>
struct id_component : component {
    static constexpr component_id_t id = ID;
};
} // namespace tos
