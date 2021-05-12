#pragma once

#include <tos/ae/rings.hpp>
#include <tos/task.hpp>

namespace tos::ae {
template<int channel_id>
struct static_channel_id {
    static constexpr auto get_channel_id() {
        return channel_id;
    }
};

struct dynamic_channel_id {
    constexpr explicit dynamic_channel_id(int channel)
        : channel_id{channel} {
    }

    constexpr auto get_channel_id() const {
        return channel_id;
    }

    int channel_id;
};

template<interface* iface, class ChannelSrc>
struct upcall_transport_channel : ChannelSrc {
    using ChannelSrc::ChannelSrc;

    tos::Task<bool> execute(int proc_id, const void* args, void* res) {
        co_await tos::ae::submit_req<false>(
            *iface, ChannelSrc::get_channel_id(), proc_id, args, res);
        co_return true;
    }
};

template<interface* iface>
struct upcall_transport {
    template<class Service, int channel>
    static auto get_service() {
        using ChannelT = upcall_transport_channel<iface, static_channel_id<channel>>;
        using StubT = typename Service::template async_zerocopy_client<ChannelT>;
        static StubT instance{};
        return &instance;
    }

    template<class Service>
    static auto get_service(int channel) {
        using ChannelT = upcall_transport_channel<iface, dynamic_channel_id>;
        using StubT = typename Service::template async_zerocopy_client<ChannelT>;
        return new StubT(channel);
    }
};
} // namespace tos::ae