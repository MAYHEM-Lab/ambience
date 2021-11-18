#pragma once

namespace tos::meta {

template<class Comparator>
struct eq {
    template<class A, class B>
    constexpr bool operator()(A a, B b) const {
        return !Comparator{}(a, b) && !Comparator{}(b, a);
    }
};
} // namespace tos::meta