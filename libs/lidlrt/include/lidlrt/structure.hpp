#pragma once

#include <cstdint>
#include <tos/fixed_string.hpp>
#include <tuple>
#include <type_traits>

namespace lidl {
template<class T>
struct struct_base {};

template<std::size_t NameLen, class T, class ConstT>
struct member_info;

template<std::size_t NameLen, class T, class RetT, class ConstRetT>
struct member_info<NameLen, RetT (T::*)(), ConstRetT (T::*)() const> {
    constexpr member_info(const char (&name)[NameLen],
                          RetT (T::*fn)(),
                          ConstRetT (T::*cfn)() const)
        : name{name}
        , function{fn}
        , const_function{cfn} {
    }
    using type = RetT;
    tos::fixed_string<NameLen> name;
    RetT (T::*function)();
    ConstRetT (T::*const_function)() const;
};

template<std::size_t N, class T, class RetT, class ConstRetT>
member_info(const char (&name)[N], RetT (T::*fn)(), ConstRetT (T::*cfn)() const)
    -> member_info<N, RetT (T::*)(), ConstRetT (T::*)() const>;

template<class T>
struct struct_traits;
} // namespace lidl

namespace lidl {
template<std::size_t I,
         class T,
         std::enable_if_t<(lidl::struct_traits<T>::arity > I)>* = nullptr>
const auto& get(const lidl::struct_base<T>& str) {
    auto& elem = static_cast<const T&>(str);
    return (elem.*(std::get<I>(lidl::struct_traits<T>::members).const_function))();
}

template<std::size_t I,
         class T,
         std::enable_if_t<(lidl::struct_traits<T>::arity > I)>* = nullptr>
auto& get(lidl::struct_base<T>& str) {
    auto& elem = static_cast<T&>(str);
    return (elem.*(std::get<I>(lidl::struct_traits<T>::members).function))();
}
} // namespace lidl

using lidl::get;

namespace std {
using lidl::get;
}