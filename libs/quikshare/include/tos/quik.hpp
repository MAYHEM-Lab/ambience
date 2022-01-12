#pragma once

#include "lidlrt/builder.hpp"
#include "tos/utility.hpp"
#include <cstdint>
#include <lidlrt/concepts.hpp>
#include <lidlrt/find_extent.hpp>
#include <lidlrt/service.hpp>
#include <string_view>
#include <tos/memory.hpp>
#include <utility>

namespace tos::quik {
struct share_base {
    virtual void* get_tuple_ptr() = 0;
    virtual ~share_base() = default;
};

template<class T>
struct sharer;

template<lidl::ValStruct T>
struct sharer<const T*> {
    static size_t compute_size(const T* obj) {
        return sizeof(T);
    }

    template<class ShareT>
    static const T* do_share(ShareT& share, const T& obj) {
        auto ptr = share.raw_allocate(sizeof(T), alignof(T));
        memcpy(ptr.direct_mapped(), &obj, sizeof(T));
        return static_cast<const T*>(ptr.direct_mapped());
    }
};

template<lidl::RefObject T>
struct sharer<const T*> {
    static size_t compute_size(const T* obj) {
        auto [extent, pos] = lidl::meta::detail::find_extent_and_position(*obj);
        return extent.size();
    }

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
    static size_t compute_size(lidl::message_builder& obj) {
        return 0;
    }

    template<class ShareT>
    static lidl::message_builder* do_share(ShareT& share,
                                           lidl::message_builder& builder) {
        auto buf = builder.get_buffer();
        auto aligned_base = virtual_address(
            align_nearest_down_pow2(reinterpret_cast<uintptr_t>(buf.data()), 4096));

        auto diff = reinterpret_cast<uintptr_t>(buf.data()) - aligned_base.address();

        auto aligned_range = virtual_range{
            aligned_base,
            align_nearest_up_pow2<ptrdiff_t>(builder.capacity() + diff, 4096)};
        auto newbase = share.map_read_write(aligned_range);

        tos::span<uint8_t> newspan(reinterpret_cast<uint8_t*>(newbase.address() + diff),
                                   builder.capacity());
        auto res = share.raw_allocate(sizeof(lidl::message_builder),
                                      alignof(lidl::message_builder));
        auto res_ptr = res.direct_mapped();
        return new (res_ptr) lidl::message_builder(newspan);
    }

    template<class ShareT>
    static void
    finalize(ShareT&, lidl::message_builder& orig, lidl::message_builder& copy) {
        orig = copy;
    }
};

template<>
struct sharer<std::string_view> {
    static size_t compute_size(const std::string_view& obj) {
        return obj.size();
    }

    template<class ShareT>
    static std::string_view do_share(ShareT& share, const std::string_view& sv_in_from) {
        auto base = share.map_read_only(
            virtual_range{virtual_address(reinterpret_cast<uintptr_t>(sv_in_from.data())),
                          static_cast<ptrdiff_t>(sv_in_from.size())});
        return std::string_view(reinterpret_cast<const char*>(base.address()),
                                sv_in_from.size());
    }
};

template<class T>
struct sharer<tos::span<T>> {
    static size_t compute_size(const tos::span<T>& obj) {
        return obj.size_bytes();
    }

    template<class ShareT>
    static tos::span<T> do_share(ShareT& share, const tos::span<T>& data) {
        auto base = share.map_read_only(
            virtual_range{virtual_address(reinterpret_cast<uintptr_t>(data.data())),
                          static_cast<ptrdiff_t>(data.size_bytes())});
        return tos::span<T>(reinterpret_cast<T*>(base.address()), data.size());
    }
};

template<class T>
struct verbatim_sharer {
    constexpr static size_t compute_size(const T&) {
        return 0;
    }

    template<class ShareT>
    static T do_share(ShareT& share, const T& arg) {
        return arg;
    }
};

template<>
struct sharer<bool> : verbatim_sharer<bool> {};
template<>
struct sharer<float> : verbatim_sharer<float> {};
template<>
struct sharer<double> : verbatim_sharer<double> {};

template<>
struct sharer<int64_t> : verbatim_sharer<int64_t> {};
template<>
struct sharer<int32_t> : verbatim_sharer<int32_t> {};
template<>
struct sharer<int16_t> : verbatim_sharer<int16_t> {};
template<>
struct sharer<int8_t> : verbatim_sharer<int8_t> {};

template<>
struct sharer<uint64_t> : verbatim_sharer<uint64_t> {};
template<>
struct sharer<uint32_t> : verbatim_sharer<uint32_t> {};
template<>
struct sharer<uint16_t> : verbatim_sharer<uint16_t> {};
template<>
struct sharer<uint8_t> : verbatim_sharer<uint8_t> {};

template<class T>
struct sharer<const T> : sharer<T> {};

namespace detail {
template<class... DataPtrTs, std::size_t... Is>
size_t compute_size(const std::tuple<DataPtrTs...>& in_ptrs, std::index_sequence<Is...>) {
    return sizeof in_ptrs +
           (sharer<DataPtrTs>::compute_size(lidl::extract(std::get<Is>(in_ptrs))) + ...);
}

template<class ShareT, class... DataPtrTs, std::size_t... Is>
auto perform_share(ShareT& share,
                   const std::tuple<DataPtrTs...>& in_ptrs,
                   std::index_sequence<Is...>) {
    return share.template allocate<std::tuple<DataPtrTs...>>(
        sharer<DataPtrTs>::do_share(share, lidl::extract(std::get<Is>(in_ptrs)))...);
}

template<class ShareT, class... DataPtrTs, std::size_t... Is>
auto finalize(ShareT& share,
              const std::tuple<DataPtrTs...>& in_ptrs,
              const std::tuple<DataPtrTs...>& out_ptrs,
              std::index_sequence<Is...>) {
    auto l = []<class DataPtrT, class T>(ShareT& share, T& i, T& o) {
        if constexpr (requires(DataPtrT) { sharer<DataPtrT>::finalize(share, i, o); }) {
            sharer<DataPtrT>::finalize(share, i, o);
        }
    };
    (l.template operator()<DataPtrTs>(share,
       lidl::extract(std::get<Is>(in_ptrs)),
       lidl::extract(std::get<Is>(out_ptrs))),
     ...);
}
} // namespace detail

template<class... DataPtrTs>
size_t compute_size(const std::tuple<DataPtrTs...>& in_ptrs) {
    return detail::compute_size(in_ptrs,
                                std::make_index_sequence<sizeof...(DataPtrTs)>{});
}

template<class ShareT, class... DataPtrTs>
auto finalize(ShareT& share,
              const std::tuple<DataPtrTs...>& in_ptrs,
              const std::tuple<DataPtrTs...>& out_ptrs) {
    if constexpr (sizeof...(DataPtrTs) == 0) {
        return;
    }

    return detail::finalize(
        share, in_ptrs, out_ptrs, std::make_index_sequence<sizeof...(DataPtrTs)>{});
}

template<class ShareT, class... DataPtrTs>
auto perform_share(ShareT& share, const std::tuple<DataPtrTs...>& in_ptrs) {
    if constexpr (sizeof...(DataPtrTs) == 0) {
        return static_cast<std::tuple<>*>(nullptr);
    }
    return detail::perform_share(
        share, in_ptrs, std::make_index_sequence<sizeof...(DataPtrTs)>{});
}
} // namespace tos::quik