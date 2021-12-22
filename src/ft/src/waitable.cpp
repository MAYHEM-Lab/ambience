#include <tos/waitable.hpp>
#include <tos/ft.hpp>
#include <tos/debug/assert.hpp>
#include <tos/debug/debug.hpp>

namespace tos {
void waitable::wait(const int_guard& ni) {
    Assert(self() && "wait must be called from a thread!");
    add(*self());
    kern::suspend_self(ni);
}

bool waitable::wait(const int_guard& ig, cancellation_token& cancel) {
    bool res = false;

    auto wait_handle = add(*self());

    auto cancel_cb = [&] {
        res = true;
        auto& t = remove(wait_handle);
        kern::make_runnable(t);
    };

    cancel.set_cancel_callback(tos::function_ref<void()>(cancel_cb));

    kern::suspend_self(ig);

    return res;
}
} // namespace tos
