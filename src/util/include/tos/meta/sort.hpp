#pragma once

#include <tos/meta/merge.hpp>
#include <tos/meta/take.hpp>

namespace tos::meta {
template<class List, class Comparator>
struct sort_list;

template<class... Ts>
using sort_list_t = typename sort_list<Ts...>::type;

template<class Comparator>
struct sort_list<list<>, Comparator> {
    using type = list<>;
};

template<class Comparator>
struct sort_list<values<>, Comparator> {
    using type = values<>;
};

template<auto A, class Comparator>
struct sort_list<values<A>, Comparator> {
    using type = values<A>;
};

template<class A, class Comparator>
struct sort_list<list<A>, Comparator> {
    using type = list<A>;
};

template<auto A, auto B, class Comparator>
struct sort_list<values<A, B>, Comparator> {
    using type = std::conditional_t<Comparator{}(A, B), values<A, B>, values<B, A>>;
};

template<class A, class B, class Comparator>
struct sort_list<list<A, B>, Comparator> {
    using type =
        std::conditional_t<Comparator{}(id<A>{}, id<B>{}), list<A, B>, list<B, A>>;
};

template<auto... Types, class Comparator>
struct sort_list<values<Types...>, Comparator> {
    static constexpr auto first_size = sizeof...(Types) / 2;
    using split = take<first_size, values<Types...>>;
    using type = merge_t<sort_list_t<typename split::type, Comparator>,
                         sort_list_t<typename split::rest, Comparator>,
                         Comparator>;
};

template<class... Types, class Comparator>
struct sort_list<list<Types...>, Comparator> {
    static constexpr auto first_size = sizeof...(Types) / 2;
    using split = take<first_size, list<Types...>>;
    using type = merge_t<sort_list_t<typename split::type, Comparator>,
                         sort_list_t<typename split::rest, Comparator>,
                         Comparator>;
};
} // namespace tos::meta