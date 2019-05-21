//
// Created by fatih on 6/26/18.
//

#include <arch/timer.hpp>
#include <tos/arch.hpp>

extern "C"
{
#include <osapi.h>
#include <user_interface.h>
}

namespace tos
{
    namespace esp82
    {
        ICACHE_FLASH_ATTR
        timer::timer() : m_cb(+[](void*){}, nullptr)
        {
            system_timer_reinit();
        }

        void ICACHE_FLASH_ATTR
        timer::set_callback(const tos::function_ref<void()> &cb) {
            m_cb = cb;
        }

        void timer::set_frequency(uint16_t hertz) {
            m_freq = hertz;
        }

        void timer::enable() {
            os_timer_setfn(&m_timer, [] (void* arg){
                auto self = static_cast<timer*>(arg);
                self->m_cb();
                system_os_post(main_task_prio, 0, 0);
            }, this);

            os_timer_arm_us(&m_timer, 1'000'000 / m_freq, true);
        }

        void timer::disable() {
            os_timer_disarm(&m_timer);
        }
    }
}