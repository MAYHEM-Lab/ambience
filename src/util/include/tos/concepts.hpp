#pragma once

#include <tos/type_traits.hpp>
#include <type_traits>

namespace tos {
template<class T, class U>
concept IsSame = std::is_same<T, U>::value;

template<class T, class U>
concept SameAs = std::is_same<T, U>::value;

template<class T, template<class...> class Template>
concept InstantiationOf = is_instantiation<Template, T>::value;

template<class Derived, class Base>
concept DerivedFrom = std::is_base_of_v<Base, Derived> &&
    std::is_convertible_v<const volatile Derived*, const volatile Base*>;

template <class T>
concept Enumeration = std::is_enum_v<T>;
} // namespace tos