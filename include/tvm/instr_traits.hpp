//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/traits.hpp>
#include <type_traits>
#include <utility>

template <class... T>
constexpr uint8_t instruction_len_bits(list<T...>)
{
    return (7 + ... + operand_size_v<T>);
}

template <class T>
constexpr uint8_t instruction_len_bits()
{
    using traits = function_traits<T>;
    using args = tail_t<typename traits::arg_ts>;
    return instruction_len_bits(args{});
}

template <class T>
constexpr T ceil(float num)
{
    return (static_cast<float>(static_cast<int32_t>(num)) == num)
           ? static_cast<int32_t>(num)
           : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

template <class T>
constexpr uint8_t instruction_len()
{
    return ceil<uint8_t>(instruction_len_bits<T>() / 8.f);
}

template <class T>
constexpr uint8_t offset_bits()
{
    return instruction_len<T>() * 8 - instruction_len_bits<T>();
}

template<class...>
struct offsets;

template <class T>
struct offsets<list<T>>
{
    using type = std::index_sequence <0>;
};

template <class>
struct last_t;

template <size_t Last, size_t... rest>
struct last_t<std::index_sequence<Last, rest...>>
{
    static constexpr auto val = Last;
};

template <class T>
constexpr auto last_v = last_t<T>::val;

template <size_t, class>
struct merge;

template <size_t top, size_t... rem>
struct merge<top, std::index_sequence<rem...>>
{
    using type = std::index_sequence<top, rem...>;
};

template <class... T, class U, class K>
struct offsets<list<U, K, T...>>
{
    using remain = typename offsets<list<K, T...>>::type;
    using type = typename merge<last_v<remain> + operand_traits<K>::size, remain>::type;
};

template <auto> struct instr_name;

template <auto Fun>
inline constexpr auto& instr_name_v = instr_name<Fun>::value;