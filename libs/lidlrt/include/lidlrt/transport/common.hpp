#pragma once

#include <lidlrt/builder.hpp>

namespace lidl {
struct verbatim_transform {
    template<class FnT>
    auto& transform_call(lidl::message_builder& mb, const FnT& create_call){
        return create_call();
    }

    template<class RetT>
    const RetT& transform_return(tos::span<const uint8_t> buf) {
        return lidl::get_root<RetT>(buf);
    }
};
}