//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <tos/utility.hpp>

namespace tos
{
    template <class T, int N> struct dev {};

    template<class... Ts> auto open_impl(Ts&&...);

    namespace details {
        template <class, class, class... Args>
        struct open_impl_exists : tos::false_type {};
        template <class... Args>
        struct open_impl_exists<void, tos::void_t<decltype(open_impl( declval<Args>()...) )>, Args...>
        : tos::true_type {};
    }

    template <class... Args>
    using open_impl_exists =typename details::open_impl_exists<void, void, Args...>::type;

    template <class T, class... ArgTs,
            typename enable_if<open_impl_exists<T, ArgTs...>{}>::type* = nullptr>
    static auto open(T t, ArgTs&&... args)
        -> decltype(open_impl(t, tos::forward<ArgTs>(args)...))
    {
        return open_impl(t, tos::forward<ArgTs>(args)...);
    }

    template <class T, class... ArgTs,
            typename disable_if<open_impl_exists<T, ArgTs...>{}>::type* = nullptr>
    static auto open(T t, ArgTs&&... args)
    {
        static_assert(open_impl_exists<T,ArgTs...>{}, "Can't open that device");
        return open_impl(t, tos::forward<ArgTs>(args)...);
    }
}
