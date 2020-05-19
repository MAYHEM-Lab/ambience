#pragma once

#include <string_view>
#include "component.hpp"

namespace tos {
struct name_component : id_component<2> {
    std::string_view name;
};
}