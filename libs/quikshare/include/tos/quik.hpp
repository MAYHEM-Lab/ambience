#pragma once

#include <cstdint>
#include <string_view>
#include <utility>
#include <lidlrt/service.hpp>

namespace tos::quik {
struct share_base {
    virtual void* get_tuple_ptr() = 0;
    virtual ~share_base() = default;
};

template<class T>
struct sharer;

template<class T>
struct sharer<const T> : sharer<T> {};

template<>
struct sharer<std::string_view> {
    template<class ShareT>
    static std::string_view* do_share(ShareT& share, const std::string_view& sv_in_from) {
        auto ptr = share.raw_allocate(sv_in_from.size(), 1);
        memcpy(ptr, sv_in_from.data(), sv_in_from.size());
        return share.template allocate<std::string_view>(static_cast<const char*>(ptr),
                                                         sv_in_from.size());
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
    share.ptrs = std::tuple<DataPtrTs...>(
        sharer<DataPtrTs>::do_share(share, lidl::extract(std::get<Is>(in_ptrs)))...);
}
} // namespace detail

template<class ShareT, class... DataPtrTs>
auto perform_share(ShareT& share, const std::tuple<DataPtrTs...>& in_ptrs) {
    return detail::perform_share(
        share, in_ptrs, std::make_index_sequence<sizeof...(DataPtrTs)>{});
}
} // namespace tos::quik