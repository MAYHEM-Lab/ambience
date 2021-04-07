#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace tos::meta {
template <class T1, class T2>
auto class_type(T1 T2::*) -> T2;

template <auto PtrToMem>
int offset_of() {
    using class_t = decltype(class_type(PtrToMem));
    return reinterpret_cast<intptr_t>(&(static_cast<class_t*>(nullptr)->*PtrToMem));
}
} // namespace tos::meta