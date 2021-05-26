#pragma once

#include <lidlrt/lidl.hpp>
#include <tos/caplets.hpp>

namespace tos::ae {
template<caplets::Token TokenType, class Base>
struct caplets_adapter : Base {
    using Base::Base;

    using frame_type = typename caplets::token_traits<TokenType>::frame_type;
    using capabilities_type =
        typename caplets::frame_traits<frame_type>::capabilities_type;
    using constraints_type = typename caplets::frame_traits<frame_type>::constraints_type;

    template<class FnT>
    auto& transform_call(lidl::message_builder& mb, const FnT& create_call) {
        std::vector<frame_type*> frames;
        auto& new_frame = lidl::create<frame_type>(
            mb,
            lidl::create_vector<capabilities_type>(
                mb, lidl::create<capabilities_type>(mb, create_call())),
            lidl::create_vector<constraints_type>(mb));

        //        auto root_space = mb.allocate(m_token_buf.size(), 1);
        //        memcpy(root_space, m_token_buf.data(), m_token_buf.size());
        //        auto& root = lidl::get_root<TokenType>(mb.get_buffer());
        //
        //        for (auto& frame : root.frames()) {
        //            frames.push_back(&frame);
        //        }
        frames.push_back(&new_frame);

        auto& mb_frames = lidl::create<TokenType>(
            mb, caplets::Tag{{}}, lidl::create_vector<frame_type>(mb, frames));
        mb_frames.tag().tag().fill(0xff);
        return mb_frames;
    }

    std::vector<uint8_t> m_token_buf;

    template<class RetT>
    const RetT& transform_return(tos::span<const uint8_t> buf) {
        return lidl::get_root<RetT>(buf);
    }
};
} // namespace tos::ae