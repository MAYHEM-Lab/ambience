#pragma once

#include <type_traits>

namespace tos::util {
template<class EnumT>
bool is_flag_set(EnumT all, EnumT look_for) {
    return (std::underlying_type_t<EnumT>(all) &
            std::underlying_type_t<EnumT>(look_for)) ==
           std::underlying_type_t<EnumT>(look_for);
}

template<class EnumT>
EnumT set_flag(EnumT all, EnumT to_set) {
    return EnumT(std::underlying_type_t<EnumT>(all) |
                 std::underlying_type_t<EnumT>(to_set));
}
} // namespace tos::util