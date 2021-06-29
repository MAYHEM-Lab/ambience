#pragma once

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <common/clock.hpp>
#include <tos/arch.hpp>
#include <tos/paging/physical_page_allocator.hpp>

tos::physical_page_allocator* initialize_page_allocator();

enum class usb_errors
{
    hci_init_fail,
    no_device
};

tos::expected<void, usb_errors> usb_task(tos::raspi3::interrupt_controller& ic,
                                         tos::any_clock& clk,
                                         tos::any_alarm& alarm);
