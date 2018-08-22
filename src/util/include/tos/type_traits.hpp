//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

namespace tos {
    namespace std {
        template<class T>
        struct remove_reference
        {
            using type = T;
        };
        template<class T>
        struct remove_reference<T&>
        {
            using type = T;
        };
        template<class T>
        struct remove_reference<T&&>
        {
            using type = T;
        };
        template<class T>
        using remove_reference_t = typename remove_reference<T>::type;

        template<class T>
        struct remove_const
        {
            using type = T;
        };

        template<class T>
        struct remove_const<const T>
        {
            using type = T;
        };

        template<class T>
        using remove_const_t = typename remove_const<T>::type;

        template<class T>
        struct is_lvalue_reference
        {
            static constexpr bool value = false;

            constexpr explicit operator bool() const
            {
                return value;
            }
        };

        template<class T>
        struct is_lvalue_reference<T&>
        {
            static constexpr bool value = true;
        };

        template<bool B, class T = void>
        struct disable_if
        {
        };

        template<class T>
        struct disable_if<false, T>
        {
            typedef T type;
        };

        template<bool B, class T = void>
        struct enable_if
        {
        };

        template<class T>
        struct enable_if<true, T>
        {
            typedef T type;
        };

        template<bool B, class T = void>
        using enable_if_t = typename enable_if<B, T>::type;

        template<class T, T v>
        struct integral_constant
        {
            static constexpr T value = v;
            using value_type = T;
            typedef integral_constant type; // using injected-class-name
            constexpr operator value_type() const noexcept
            { return value; }

            constexpr value_type operator()() const noexcept
            { return value; }
        };

        template<bool B>
        using bool_constant = integral_constant<bool, B>;

        using true_type = bool_constant<true>;
        using false_type = bool_constant<false>;

        template<class A, class B>
        struct is_same : std::false_type
        {
        };

        template<class T>
        struct is_same<T, T> : std::true_type
        {
        };

        template<class...>
        using void_t = void;

        template<class T>
        T&& declval();

        template <bool, class, class B>
        struct conditional_type {
            using type = B;
        };

        template <class A, class B>
        struct conditional_type<true, A, B>
        {
            using type = A;
        };

        template <bool Val, class A, class B>
        using conditional_t = typename conditional_type<Val, A, B>::type;
    }
}

