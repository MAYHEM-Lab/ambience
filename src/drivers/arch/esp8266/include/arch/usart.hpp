//
// Created by fatih on 4/26/18.
//

#pragma once

#include <c_types.h>
#include <common/driver_base.hpp>
#include <common/usart.hpp>
#include <tos/driver_traits.hpp>
#include <tos/span.hpp>
#include <tos/thread.hpp>

namespace tos {
namespace esp82 {
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

struct uart0 : self_pointing<uart0> {
public:
    uart0(usart_constraint);

    static int write(span<const char>);
};

struct sync_uart0 {
    static int write(span<const char>);
};

static_assert(driver_traits<uart0>::has_write{}, "uart must have write!");
static_assert(!driver_traits<uart0>::has_read{}, "uart must not have read!");
} // namespace esp82

inline esp82::uart0 open_impl(devs::usart_t<0>, esp82::usart_constraint params) {
    return {std::move(params)};
}
} // namespace tos


extern "C" {
#define UART_STOP_BITS_ONE 0x10
#define UART_STOP_BITS_ONE_HALF 0x20
#define UART_STOP_BITS_TWO 0x30
#define UART_STOP_BITS_MASK 0x30

#define UART_BITS_FIVE 0x00
#define UART_BITS_SIX 0x04
#define UART_BITS_SEVEN 0x08
#define UART_BITS_EIGHT 0x0C
#define UART_BITS_MASK 0x0C

#define UART_PARITY_EVEN 0x00
#define UART_PARITY_ODD 0x01
#define UART_PARITY_NONE 0x02
#define UART_PARITY_MASK 0x03

#define UART_FLAGS_8N1 (UART_BITS_EIGHT | UART_PARITY_NONE | UART_STOP_BITS_ONE)

#define UART_STATUS_IDLE 0x00
#define UART_STATUS_RECEIVING 0x01
#define UART_STATUS_OVERFLOW 0x02

void uart0_open(uint32 baud_rate, uint32 flags);
void uart0_reset();

uint16 uart0_available();
uint16 uart0_read_buf(void* buf, uint16 nbyte, uint16 timeout);
uint16 uart0_write_buf(const void* buf, uint16 nbyte, uint16 timeout);
void uart0_tx_buffer(const uint8* buf, uint16 len);
void uart0_flush();
void uart0_tx_buffer_sync(const uint8_t* buf, uint16_t len);

void uart1_open(uint32 baud_rate, uint32 flags);
void uart1_reset(uint32 baud_rate, uint32 flags);
uint8 uart1_write_byte(uint8 byte);
}

namespace tos {
namespace esp82 {
inline uart0::uart0(usart_constraint params) {
    ::uart0_open(get<usart_baud_rate>(params).rate, UART_FLAGS_8N1);
}

inline int ICACHE_FLASH_ATTR uart0::write(tos::span<const char> buf) {
    ::uart0_tx_buffer((uint8*)buf.data(), buf.size());
    return buf.size();
}

inline int sync_uart0::write(span<const char> buf) {
    ::uart0_tx_buffer_sync((const uint8_t*)buf.data(), buf.size());
    return buf.size();
}
} // namespace esp82
} // namespace tos