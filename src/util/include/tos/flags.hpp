#pragma once

#include <tos/concepts.hpp>
#include <type_traits>

namespace tos::util {
template<Enumeration EnumT>
constexpr bool is_flag_set(EnumT all, EnumT look_for) {
    return (std::underlying_type_t<EnumT>(all) &
            std::underlying_type_t<EnumT>(look_for)) ==
           std::underlying_type_t<EnumT>(look_for);
}

template<Enumeration EnumT>
constexpr EnumT set_flag(EnumT all, EnumT to_set) {
    return EnumT(std::underlying_type_t<EnumT>(all) |
                 std::underlying_type_t<EnumT>(to_set));
}

template<Enumeration EnumT>
constexpr EnumT clear_flag(EnumT all, EnumT clear) {
    return EnumT(std::underlying_type_t<EnumT>(all) &
                 ~std::underlying_type_t<EnumT>(clear));
}
} // namespace tos::util