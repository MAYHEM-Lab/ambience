#pragma once

#include "lidlrt/builder.hpp"
#include "tos/utility.hpp"
#include <cstdint>
#include <lidlrt/concepts.hpp>
#include <lidlrt/find_extent.hpp>
#include <lidlrt/service.hpp>
#include <string_view>
#include <utility>

namespace tos::quik {
struct share_base {
    virtual void* get_tuple_ptr() = 0;
    virtual ~share_base() = default;
};

template<class T>
struct sharer;

template<lidl::RefObject T>
struct sharer<const T*> {
    template<class ShareT>
    static const T* do_share(ShareT& share, const T* obj) {
        auto [extent, pos] = lidl::meta::detail::find_extent_and_position(*obj);
        auto ptr = share.raw_allocate(extent.size(), 1);
        memcpy(ptr.direct_mapped(), extent.data(), extent.size());
        return reinterpret_cast<const T*>(static_cast<const char*>(ptr.direct_mapped()) +
                                          pos);
    }
};

template<>
struct sharer<lidl::message_builder*> {
    template<class ShareT>
    static lidl::message_builder* do_share(ShareT& share,
                                           lidl::message_builder* builder) {
        auto buf = builder->get_buffer();
        share.map_read_write(
            align_nearest_down_pow2(reinterpret_cast<uintptr_t>(buf.data()), 4096));
        // This should not be mapped, but a new one cretead!
        share.map_read_write(
            align_nearest_down_pow2(reinterpret_cast<uintptr_t>(builder), 4096));
        return builder;
    }
};

template<class T>
struct sharer<const T> : sharer<T> {};

template<>
struct sharer<std::string_view> {
    template<class ShareT>
    static std::string_view do_share(ShareT& share, const std::string_view& sv_in_from) {
        auto ptr = share.raw_allocate(sv_in_from.size(), 1);
        memcpy(ptr.direct_mapped(), sv_in_from.data(), sv_in_from.size());
        return std::string_view(static_cast<const char*>(ptr.direct_mapped()),
                                sv_in_from.size());
    }
};

template<class T>
struct sharer<tos::span<T>> {
    template<class ShareT>
    static tos::span<T> do_share(ShareT& share, const tos::span<T>& data) {
        LOG("Sharing span", data);
        auto ptr = share.raw_allocate(data.size(), 1);
        memcpy(ptr.direct_mapped(), data.data(), data.size());
        return tos::span<T>(static_cast<T*>(ptr.direct_mapped()), data.size());
    }
};

template<class T>
struct verbatim_sharer {
    template<class ShareT>
    static T do_share(ShareT& share, const T& arg) {
        return arg;
    }
};

template<>
struct sharer<double> : verbatim_sharer<double> {};
template<>
struct sharer<int64_t> : verbatim_sharer<int64_t> {};
template<>
struct sharer<int32_t> : verbatim_sharer<int32_t> {};

namespace detail {
template<class ShareT, class... DataPtrTs, std::size_t... Is>
auto perform_share(ShareT& share,
                   const std::tuple<DataPtrTs...>& in_ptrs,
                   std::index_sequence<Is...>) {
    return share.template allocate<std::tuple<DataPtrTs...>>(
        sharer<DataPtrTs>::do_share(share, lidl::extract(std::get<Is>(in_ptrs)))...);
}
} // namespace detail

template<class ShareT, class... DataPtrTs>
auto perform_share(ShareT& share, const std::tuple<DataPtrTs...>& in_ptrs) {
    return detail::perform_share(
        share, in_ptrs, std::make_index_sequence<sizeof...(DataPtrTs)>{});
}
} // namespace tos::quik