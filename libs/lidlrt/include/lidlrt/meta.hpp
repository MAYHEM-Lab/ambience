#pragma once

#include <tuple>

namespace lidl::meta {
template<class...>
struct list {};

template<class T>
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

template<class T, class Tuple>
struct list_index_of;

template<class T, class... Types>
struct list_index_of<T, list<T, Types...>> {
    static const std::size_t value = 0;
};

template<class T, class U, class... Types>
struct list_index_of<T, list<U, Types...>> {
    static const std::size_t value = 1 + tuple_index_of<T, list<Types...>>::value;
};

template<class T>
using remove_cref = std::remove_const_t<std::remove_reference_t<T>>;

template <class, template <class, class...> class>
struct is_instance : public std::false_type {};
template <class...Ts, template <class, class...> class U>
struct is_instance<U<Ts...>, U> : public std::true_type {};
} // namespace lidl::meta