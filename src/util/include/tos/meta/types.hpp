#pragma once

namespace tos::meta {
template<class T>
struct identity {
    using type = T;
};

template<class T>
using id = identity<T>;

template<class...>
struct list {};
} // namespace tos::meta