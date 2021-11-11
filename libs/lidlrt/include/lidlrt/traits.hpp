#pragma once

#include <lidlrt/structure.hpp>
#include <type_traits>

namespace lidl {
template <class T>
class ptr;
template <class T> struct union_base;

template <class T>
struct is_ptr : std::false_type {};

template <class T>
struct is_ptr<ptr<T>> : std::true_type {};

template <class T>
struct is_reference_type : std::false_type {};

template <class T>
struct is_struct : std::is_base_of<struct_base<T>, T> {};

template <class T>
struct is_union : std::is_base_of<union_base<T>, T> {};

template<class UnionT>
struct union_traits;

template <class ModuleT>
struct module_traits;

struct sync_service_base;
struct async_service_base;

template <class T>
struct is_sync_service : std::is_base_of<sync_service_base, T> {};

template <class T>
struct is_async_service : std::is_base_of<async_service_base, T> {};
}