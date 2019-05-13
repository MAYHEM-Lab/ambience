//
// Created by fatih on 12/13/18.
//

#include <arch/avr/wdt.hpp>

namespace tos
{
namespace avr
{
    tos::function_ref<void()> wdt_handler{[](void*){}};
} // namespace avr
} // namespace tos
ISR (WDT_vect)
{
    tos::avr::wdt_handler();
}
