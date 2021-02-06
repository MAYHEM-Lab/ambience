#pragma once

#include <tos/function_ref.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/debug/assert.hpp>

namespace tos {
class cancellation_token : public list_node<cancellation_token> {
public:
    explicit cancellation_token(cancellation_token& parent) {
        auto list = ad_hoc_list(parent);
        list.push_back(*this);
        m_parent = &parent;
    }

    cancellation_token(const cancellation_token&) = delete;
    cancellation_token(cancellation_token&&) = delete;

    void cancel() {
        Assert(!is_cancelled());
#ifdef TOSDEBUG
        Assert(this != &never());
#endif
        auto list = ad_hoc_list(*this);
        auto it = ad_hoc_list_iter(*this);
        auto this_iter = it++;

        do_cancel(list, it);

        list.erase(this_iter);
    }

    void set_cancel_callback(function_ref<void()> handler) {
        m_callback = handler;
    }

    [[nodiscard]] bool is_cancelled() const {
        return m_cancelled;
    }

    static cancellation_token& never() {
        static cancellation_token tok{};
        return tok;
    }

    static cancellation_token& system() {
        static cancellation_token tok{};
        return tok;
    }

    cancellation_token nest() {
        return cancellation_token(*this);
    }

    ~cancellation_token() {
        if (m_cancelled)
            return;
        ad_hoc_list(*this).erase(ad_hoc_list_iter(*this));
    }

private:
    void do_cancel(intrusive_list<cancellation_token>& list,
                   intrusive_list<cancellation_token>::iterator_t it) {
        for (; it != list.end();) {
            if (it->m_parent == this) {
                it->do_cancel(list, it);
                it = list.erase(it);
                continue;
            }
            ++it;
        }

        m_cancelled = true;
        m_callback();
    }

    cancellation_token() {
        prev = nullptr;
        next = nullptr;
    }

    cancellation_token* m_parent = nullptr;
    function_ref<void()> m_callback{[](void*) {}};
    bool m_cancelled = false;
};

inline tos::function_ref<void()> make_canceller(cancellation_token& tok) {
    return mem_function_ref<&cancellation_token::cancel>(tok);
}
} // namespace tos