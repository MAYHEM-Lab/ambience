#pragma once

#include <lidlrt/meta.hpp>
#include <lidlrt/traits.hpp>

namespace lidl {
template<class T>
struct union_base {};

namespace detail {
template<class Type, class UnionType>
constexpr auto index_of() -> typename UnionType::alternatives {
    using traits = union_traits<UnionType>;
    using types = typename traits::types;
    return static_cast<typename UnionType::alternatives>(
        meta::list_index_of<Type, types>::value);
}
} // namespace detail

template<class Type, class UnionType>
Type* get_if(UnionType* u) {
    constexpr auto variant = detail::index_of<Type, UnionType>();

    constexpr auto& mems = union_traits<UnionType>::members;

    if (u->alternative() != variant) {
        return nullptr;
    }

    return &std::invoke(std::get<static_cast<int>(variant)>(mems).function, *u);
}

template<class Type, class UnionType>
const Type* get_if(const UnionType* u) {
    constexpr auto variant = detail::index_of<Type, UnionType>();

    constexpr auto& mems = union_traits<UnionType>::members;

    if (u->alternative() != variant) {
        return nullptr;
    }

    return &std::invoke(std::get<static_cast<int>(variant)>(mems).const_function, *u);
}
} // namespace lidl