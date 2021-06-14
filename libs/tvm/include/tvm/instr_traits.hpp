//
// Created by fatih on 4/20/18.
//

#pragma once

#include <cstddef>
#include <tvm/operand_traits.hpp>
#include <tvm/tvm_types.hpp>
#include <tvm/util/array.hpp>
#include <utility>

using std::index_sequence;
using std::integral_constant;
using std::tuple;

namespace tvm {
constexpr uint8_t instruction_len_bits(list<>) {
    return 0;
}

template<class T, class... Tail>
constexpr uint8_t instruction_len_bits(list<T, Tail...>) {
    return operand_size_v<T> + instruction_len_bits(list<Tail...>{});
}

template<class T, uint8_t N>
constexpr uint8_t instruction_len_bits() {
    using traits = functor_traits<T>;
    using args = tail_t<typename traits::arg_ts>;
    return instruction_len_bits(args{}) + operand_size_v<opcode_t<N>>;
}

template<class T>
constexpr int8_t operand_count() {
    using traits = functor_traits<T>;
    using args = tail_t<typename traits::arg_ts>;
    return len(args{});
}

template<class T>
constexpr T ceil(float num) {
    return (static_cast<float>(static_cast<int32_t>(num)) == num)
               ? static_cast<int32_t>(num)
               : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

template<class T, uint8_t N>
constexpr uint8_t instruction_len() {
    return ceil<uint8_t>(instruction_len_bits<T, N>() / 8.f);
}

template<class T, uint8_t N>
constexpr uint8_t offset_bits() {
    return instruction_len<T, N>() * 8 - instruction_len_bits<T, N>();
}

template<class...>
struct offsets;

template<>
struct offsets<list<>> {
    using type = std::index_sequence<>;
};

template<class T>
struct offsets<list<T>> {
    using type = std::index_sequence<0>;
};

template<class>
struct last_t;

template<size_t Last, size_t... rest>
struct last_t<std::index_sequence<Last, rest...>> {
    static constexpr auto val = Last;
};

template<class T>
constexpr auto last_v = last_t<T>::val;

template<size_t, class>
struct merge;

template<size_t top, size_t... rem>
struct merge<top, std::index_sequence<rem...>> {
    using type = std::index_sequence<top, rem...>;
};

template<class... T, class U, class K>
struct offsets<list<U, K, T...>> {
    using remain = typename offsets<list<K, T...>>::type;
    using type = typename merge<last_v<remain> + operand_traits<K>::size, remain>::type;
};

template<size_t... offsets, int index>
constexpr size_t offset_at(std::index_sequence<offsets...>,
                           std::integral_constant<int, index>) {
    return 0;
}

template<size_t... offsets>
constexpr array<size_t, sizeof...(offsets)> to_array(std::index_sequence<offsets...>) {
    return {{offsets...}};
}

template<class T, int OpInd>
constexpr auto get_operand_offset() {
    using traits = functor_traits<T>;
    using args = tail_t<typename traits::arg_ts>;
    using offsets_ = offsets<args>;
    return offset_at(typename offsets_::type{}, std::integral_constant<int, OpInd>{});
}

template<class InstrT>
struct instr_name {
    static constexpr auto value() {
        return InstrT::name;
    }
};

template<class Fun>
constexpr auto&& instr_name_v = instr_name<Fun>::value();

template<class T, int OpInd>
constexpr auto get_operand_at() {
    using traits = functor_traits<T>;
    using args = tail_t<typename traits::arg_ts>;
    return identity<type_at_t<OpInd + 1, args>>{};
}
} // namespace tvm