#include <tos/interrupt.hpp>

namespace tos::global {
int8_t disable_depth = 1;
bool should_enable = true;
}