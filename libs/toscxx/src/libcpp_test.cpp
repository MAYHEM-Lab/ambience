/**
 * This executable makes sure the standard C++ library implementation compiles and links
 * successfully.
 */

#include <tuple>
#include <functional>
#include <algorithm>
#include <vector>
#include <array>
#include <string>
#include <variant>
#include <optional>
#include <chrono>
#include <map>
#include <forward_list>
#include <unordered_map>
#include <memory>

#include <tos/compiler.hpp>
#include <tos/debug/debug.hpp>

NO_INLINE
std::string get_string() {
    return std::string(100, 'h');
}

NO_INLINE
std::shared_ptr<std::string> get_shared_ptr() {
    return std::make_shared<std::string>(100, 'h');
}

void tos_main() {
    tos::debug::do_not_optimize(get_string());
    tos::debug::do_not_optimize(get_shared_ptr());
}