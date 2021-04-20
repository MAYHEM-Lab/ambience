#pragma once

#include <tos/function_ref.hpp>
#include <tos/self_pointing.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::x86_64 {
class pit : public self_pointing<pit> {
public:
    pit();

    void set_frequency(int freq);
    void set_callback(function_ref<void()> cb);

    void enable();
    void disable();

private:

    void irq(tos::x86_64::exception_frame* frame, int);

    function_ref<void()> m_cb;
};
} // namespace tos::x86_64