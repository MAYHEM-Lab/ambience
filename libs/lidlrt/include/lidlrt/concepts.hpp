#pragma once

#include <lidlrt/service.hpp>
#include <lidlrt/union.hpp>
#include <lidlrt/meta.hpp>
#include <type_traits>

namespace lidl {
template <class T>
concept Ref = is_reference_type<T>{};

template <class T>
concept Value = !Ref<T>;

template <class T>
concept Struct = is_struct<T>{};

template <class T>
concept ValStruct = Struct<T> && Value<T>;

template <class T>
concept RefStruct = Struct<T> && Ref<T>;

template <class T>
concept Union = is_union<T>{} && requires (T t) {
    { t.alternative() };
};

template <class T>
concept ValUnion = Union<T> && Value<T>;

template <class T>
concept RefUnion = Union<T> && Ref<T>;
}