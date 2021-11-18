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
} // namespace tos