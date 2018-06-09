//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <tos/utility.hpp>

namespace tos
{
    /**
     * Tag type for devices in tos.
     * @tparam T tag of the device (think of major number)
     * @tparam N number of the device (think of minor number)
     */
    template <class T, int N> struct dev {};

    namespace details {
        template <class, class, class... Args>
        struct open_impl_exists : tos::false_type {};
        template <class... Args>
        struct open_impl_exists<void, tos::void_t<decltype(open_impl( declval<Args>()...) )>, Args...>
        : tos::true_type {};
    }

    template <class... Args>
    using open_impl_exists =typename details::open_impl_exists<void, void, Args...>::type;

    /**
     * This function template opens a device denoted by the device tag argument. The tag is expected
     * to be an instance of the `tos::dev` template but it's not enforced.
     *
     * If the device doesn't exist for the current target or it can't be found for other reasons,
     * the function causes a compile-time error.
     *
     * This function is implemented in terms of another function called `open_impl` which is a
     * potentially architecture dependent operation. When writing drivers, overload `open_impl`
     * to be considered for opens.
     *
     * The reason this function exists is to provide a uniform interface to device access. In the
     * future, runtime or compile time checks could be inserted to restrict access or improve
     * correctness.
     *
     * However, it makes it difficult to find the actual open implementation for the given device,
     * both for users and IDEs. Looking into improvements on this.
     *
     * @tparam T tag type of the device to open
     * @tparam ArgTs types of arguments to pass to the actual open_impl call
     * @param t tag of the device to open
     * @param args arguments to pass to the actual open_impl call
     * @return the device object
     */
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
        static_assert(open_impl_exists<T,ArgTs...>{}, "Can't open that device, did you include necessary headers?");
        return open_impl(t, tos::forward<ArgTs>(args)...);
    }
}
