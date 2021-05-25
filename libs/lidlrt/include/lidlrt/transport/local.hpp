#pragma once

#include <vector>

namespace lidl {
template<class ServerType>
struct local_transport {
    template<class... Args>
    local_transport(Args&&... t)
        : m_serv{std::forward<Args>(t)...} {
    }

    std::vector<uint8_t> get_buffer() {
        return std::vector<uint8_t>(1024);
    }

    std::vector<uint8_t> send_receive(tos::span<uint8_t> data) {
        //        tos::debug::log(data);
        std::vector<uint8_t> buf(1024);
        lidl::message_builder mb(buf);
        m_serv.run_message(data, mb);
        buf.resize(mb.size());
        //        tos::debug::log(buf);
        return buf;
    }

    template<class FnT>
    auto& transform_call(lidl::message_builder&, const FnT& fn) {
        return fn();
    }

    template<class RetT>
    const RetT& transform_return(tos::span<const uint8_t> buf) {
        return lidl::get_root<RetT>(buf);
    }

    ServerType m_serv;
};
} // namespace lidl
