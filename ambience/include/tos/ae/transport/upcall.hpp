#pragma once

#include <tos/ae/rings.hpp>
#include <tos/task.hpp>

namespace tos::ae {
struct awaiter {
    auto operator co_await() const noexcept {
        tos::ae::submit_elem<false>(*iface, id);
        return m_elem->operator co_await();
    }

    req_elem* m_elem;
    int id;
    interface* iface;
};

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

    awaiter execute(int proc_id, const void* args, void* res) {
        const auto& [req, id] =
            prepare_req(*iface, ChannelSrc::get_channel_id(), proc_id, args, res);

        return awaiter{
            .m_elem = &req,
            .id = id,
            .iface = iface,
        };
    }
};

template<interface* iface>
struct upcall_transport {
    template<class Service, int channel>
    static auto get_service() {
        using ChannelT = upcall_transport_channel<iface, static_channel_id<channel>>;
        using StubT = typename Service::template async_zerocopy_client<ChannelT>;
        static constexpr StubT instance{};
        return const_cast<StubT*>(&instance);
    }

    template<class Service>
    static auto get_service(int channel) {
        using ChannelT = upcall_transport_channel<iface, dynamic_channel_id>;
        using StubT = typename Service::template async_zerocopy_client<ChannelT>;
        return new StubT(channel);
    }
};
} // namespace tos::ae