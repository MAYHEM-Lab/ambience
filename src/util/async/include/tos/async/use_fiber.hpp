#pragma once

#include <tos/basic_fiber.hpp>

namespace tos::async {
struct use_fiber {
    
    tos::any_fiber* m_fiber;
};
}