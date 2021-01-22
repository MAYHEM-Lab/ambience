#pragma once

namespace lidl::meta {
template<class...>
struct list {};

template <class T>
struct identity {
    using type = T;
};

template<class T, class Tuple>
struct tuple_index_of;

template<class T, class... Types>
struct tuple_index_of<T, std::tuple<T, Types...>> {
    static const std::size_t value = 0;
};

template<class T, class U, class... Types>
struct tuple_index_of<T, std::tuple<U, Types...>> {
    static const std::size_t value = 1 + tuple_index_of<T, std::tuple<Types...>>::value;
};

template<class T>
using remove_cref = std::remove_const_t<std::remove_reference_t<T>>;
}