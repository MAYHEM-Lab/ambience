#pragma once

namespace tos::meta {
template<class T>
struct identity {
    using type = T;
};

template<class T>
using id = identity<T>;
} // namespace tos::meta