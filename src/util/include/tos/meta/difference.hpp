#pragma once

#include <tos/meta/concat.hpp>
#include <tos/meta/eq.hpp>
#include <tos/meta/types.hpp>
#include <type_traits>

namespace tos::meta {
template<class A, class B, class Comparator>
struct difference;

template<class... Ts>
using difference_t = typename difference<Ts...>::type;

template<class Comparator>
struct difference<list<>, list<>, Comparator> {
    using type = list<>;
};

template<class... As, class Comparator>
struct difference<list<As...>, list<>, Comparator> {
    using type = list<As...>;
};

template<class... As, class Comparator>
struct difference<list<>, list<As...>, Comparator> {
    using type = list<As...>;
};

template<class AHead, class... As, class BHead, class... Bs, class Comparator>
struct difference<list<AHead, As...>, list<BHead, Bs...>, Comparator> {
    static constexpr auto head_eq = eq<Comparator>{}(id<AHead>{}, id<BHead>{});
    static constexpr auto a_lt = Comparator{}(id<AHead>{}, id<BHead>{});

    /**
     * If the first elements of both sorted lists are equal, then it's in the
     * intersection. Otherwise, we drop the smaller of the heads and continue
     * intersecting, since there's no way it's in the other one.
     */
    using type = std::conditional_t<
        head_eq,
        difference_t<list<As...>, list<Bs...>, Comparator>,
        std::conditional_t<
            a_lt,
            concat_t<list<AHead>,
                     difference_t<list<As...>, list<BHead, Bs...>, Comparator>>,
            concat_t<list<BHead>,
                     difference_t<list<AHead, As...>, list<Bs...>, Comparator>>>>;
};
} // namespace tos::meta