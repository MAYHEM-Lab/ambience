#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace tos::meta {
template<class T1, class T2>
auto class_type(T1 T2::*) -> T2;

template<class T1, class T2>
auto val_type(T1 T2::*) -> T1;


template<auto PtrToMem>
int offset_of() {
    using class_t = decltype(class_type(PtrToMem));
    return reinterpret_cast<intptr_t>(&(static_cast<class_t*>(nullptr)->*PtrToMem));
}

template<auto PtrToMem>
static auto& reverse_member(decltype(val_type(PtrToMem))& member) {
    using T = decltype(class_type(PtrToMem));

    auto off = meta::offset_of<PtrToMem>();
    return *reinterpret_cast<T*>(reinterpret_cast<char*>(&member) - off);
}
} // namespace tos::meta