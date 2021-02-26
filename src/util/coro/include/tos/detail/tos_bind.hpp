#pragma once

#include <tos/detail/poll.hpp>
#include <tos/job.hpp>

namespace tos {
class coro_job : public job {
public:
    coro_job(context& ctx, coro::pollable&& p)
        : job(ctx)
        , m_pollable{std::move(p)} {
    }

    void operator()() override {
        m_pollable.run();
    }

private:
    coro::pollable m_pollable;
};
} // namespace tos