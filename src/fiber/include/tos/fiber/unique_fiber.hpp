#pragma once

#include <functional>
#include <memory>
#include <tos/functional.hpp>

namespace tos {
namespace fiber::detail {
struct deleter {
    template<class T>
    void operator()(T* t) {
        t->destroy();
    }
};
} // namespace fiber::detail

template<class FibT>
using unique_fiber = std::unique_ptr<FibT, fiber::detail::deleter>;


template<class FibT>
unique_fiber<FibT> unique(FibT* ptr) {
    return unique_fiber<FibT>(ptr);
}
} // namespace tos