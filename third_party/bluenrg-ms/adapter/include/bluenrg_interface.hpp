#pragma once

#include <common/alarm.hpp>
#include <cstdint>

struct bluenrg_interface {
    virtual int32_t read(uint8_t* buffer, uint8_t buff_size) = 0;
    virtual void
    write(const void* data1, const void* data2, int32_t n_bytes1, int32_t n_bytes2) = 0;
    virtual void disable_irq() = 0;
    virtual void enable_irq() = 0;
    virtual uint8_t data_present() = 0;

    virtual tos::any_alarm* get_alarm() = 0;

    virtual void event_cb(void* pckt) = 0;

    virtual ~bluenrg_interface() = default;
};

void HCI_Init(bluenrg_interface& arg_interface);