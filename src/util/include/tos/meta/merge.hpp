#pragma once

#include <tos/meta/concat.hpp>
#include <tos/meta/types.hpp>
#include <type_traits>

namespace tos::meta {
template<class Left, class Right, class Comparator>
struct merge;

template<class... Ts>
using merge_t = typename merge<Ts...>::type;

template<auto... Bs, class Comparator>
struct merge<values<>, values<Bs...>, Comparator> {
    using type = values<Bs...>;
};

template<class... Bs, class Comparator>
struct merge<list<>, list<Bs...>, Comparator> {
    using type = list<Bs...>;
};

template<auto... As, class Comparator>
struct merge<values<As...>, values<>, Comparator> {
    using type = values<As...>;
};

template<class... As, class Comparator>
struct merge<list<As...>, list<>, Comparator> {
    using type = list<As...>;
};

template<auto AHead, auto... As, auto BHead, auto... Bs, class Comparator>
struct merge<values<AHead, As...>, values<BHead, Bs...>, Comparator> {
    using type = std::conditional_t<
        Comparator{}(AHead, BHead),
        concat_t<values<AHead>, merge_t<values<As...>, values<BHead, Bs...>, Comparator>>,
        concat_t<values<BHead>,
                 merge_t<values<AHead, As...>, values<Bs...>, Comparator>>>;
};

template<class AHead, class... As, class BHead, class... Bs, class Comparator>
struct merge<list<AHead, As...>, list<BHead, Bs...>, Comparator> {
    using type = std::conditional_t<
        Comparator{}(id<AHead>{}, id<BHead>{}),
        concat_t<list<AHead>, merge_t<list<As...>, list<BHead, Bs...>, Comparator>>,
        concat_t<list<BHead>, merge_t<list<AHead, As...>, list<Bs...>, Comparator>>>;
};
} // namespace tos::meta