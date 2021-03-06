#pragma once

#include <array>

namespace tos::x86_64::cpuid {
std::array<char, 12> manufacturer();
}