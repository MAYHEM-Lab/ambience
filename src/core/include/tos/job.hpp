#pragma once

#include <tos/intrusive_list.hpp>
#include <tos/context.hpp>

namespace tos {
class job : public list_node<job> {
public:
    explicit job(context& ctx);

    void set_context(context& ctx);
    context& get_context();

    virtual void operator()() = 0;
    virtual ~job() = default;

protected:
    virtual void on_set_context([[maybe_unused]] context& new_ctx) {}

private:
    context* m_context;
};
}