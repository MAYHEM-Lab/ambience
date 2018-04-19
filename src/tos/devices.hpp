//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <tos/utility.hpp>

namespace tos
{
    template <class T, int N> struct dev {};

    template<class... Ts>
    auto open_impl(Ts&&...);

    namespace details {
        template <class, class, class... Args>
        struct is_invokable : tos::false_type {};
        template <class... Args>
        struct is_invokable<void, tos::void_t<decltype(open_impl( declval<Args>()...) )>, Args...>
        : tos::true_type {};
    }

    template <class... Args>
    using open_impl_exists =typename details::is_invokable<void, void, Args...>::type;

    template <class T, class... ArgTs,
            typename enable_if<open_impl_exists<T, ArgTs...>{}>::type* = nullptr>
    static auto open(T t, ArgTs&&... args)
    {
        return open_impl(t, tos::forward<ArgTs>(args)...);
    }

    template <class T, class... ArgTs,
            typename disable_if<open_impl_exists<T, ArgTs...>{}>::type* = nullptr>
    static auto open(T, ArgTs&&...)
    {
        static_assert(open_impl_exists<T,ArgTs...>{}, "Can't open that device");
    }

    namespace devs
    {
        template <int N> using spi_t = dev<struct _spi_t, N>;
        template <int N> static constexpr spi_t<N> spi{};

        template <int N> using usart_t = dev<struct _usart_t, N>;
        template <int N> static constexpr usart_t<N> usart{};
    }
}
