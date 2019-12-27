//
// Created by fatih on 11/6/19.
//

#include "tos/interrupt.hpp"

#include <arch/drivers.hpp>

extern "C" {
extern const UART_Config UART_config[];
}

namespace tos::cc32xx {
void uart_driver_callback(UART_Handle handle, void* buf, size_t count) {
    tos::int_guard ig;
    auto id = std::distance(&UART_config[0], static_cast<const UART_Config*>(handle));
    uart::get(id)->isr();
}
uart::uart(int id)
    : tracked_driver(id) {
    static auto _ = [] {
        UART_init();
        return 0;
    }();
    UART_Params params;
    UART_Params_init(&params);

    params.baudRate = 115200;
    params.readEcho = UART_ECHO_OFF;

    params.writeDataMode = UART_DATA_BINARY;
    params.readDataMode = UART_DATA_BINARY;

    params.readReturnMode = UART_RETURN_FULL;

    params.writeMode = UART_MODE_CALLBACK;
    params.readMode = UART_MODE_CALLBACK;

    params.readCallback = uart_driver_callback;
    params.writeCallback = uart_driver_callback;

    m_handle = UART_open(id, &params);
}
} // namespace tos::cc32xx