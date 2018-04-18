//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

namespace tos
{
    template <class T, int N> struct dev {};

    template <class T, class... ArgTs>
    auto open(T t, ArgTs&&... args)
    {
        return open_impl(t, args...);
    }

    namespace devs
    {
        template <int N> using spi_t = dev<struct _spi_t, N>;
        template <int N> static constexpr spi_t<N> spi{};

        template <int N> using usart_t = dev<struct _usart_t, N>;
        template <int N> static constexpr usart_t<N> usart{};
    }
}
