/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Juho Vähä-Herttua (juhovh@iki.fi)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <tos/mutex.hpp>
#include <algorithm>
#include <mem.h>

#include <tos/semaphore.hpp>
#include <tos/mutex.hpp>

struct uart_state
{
    const char* tx_buffer{nullptr};
    size_t buf_sz{0};
    tos::semaphore tx_done{0};
    tos::mutex tx_busy;
};

extern "C"
{
#include <user_interface.h>
#include <arch/lx106/lx106_missing.hpp>
#include "new_uart_priv.h"

#include "osapi.h"
#include "ets_sys.h"
#include "uart_register.h"

#define WAIT_RESOLUTION_US 1000

#define UART0 0
#define UART1 1

// Missing defines from SDK
#define FUNC_U0RXD 0

// Define FIFO sizes, actually 128 but playing safe
#define UART_RXFIFO_SIZE 126
#define UART_TXFIFO_SIZE 126
#define UART_RXTOUT_TH   2
#define UART_RXFIFO_TH   100
#define UART_TXFIFO_TH   1

// Define some helper macros to handle FIFO functions
#define UART_TXFIFO_LEN(uart_no) \
  ((READ_PERI_REG(UART_STATUS(uart_no)) >> UART_TXFIFO_CNT_S) & UART_RXFIFO_CNT)
#define UART_TXFIFO_PUT(uart_no, byte) \
  WRITE_PERI_REG(UART_FIFO(uart_no), (byte) & 0xff)
#define UART_RXFIFO_LEN(uart_no) \
  ((READ_PERI_REG(UART_STATUS(uart_no)) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT)
#define UART_RXFIFO_GET(uart_no) \
  READ_PERI_REG(UART_FIFO(uart_no))


#define RINGBUF_SIZE 512
struct ringbuf_s {
    uint8 data[RINGBUF_SIZE];
    uint16 head;
    uint16 len;
};
typedef struct ringbuf_s ringbuf_t;

#define RINGBUF_CLEAR(buf) ((buf)->head = (buf)->len = 0)
#define RINGBUF_TAIL(buf) (((buf)->head + (buf)->len) % RINGBUF_SIZE)
#define RINGBUF_PUT(buf, byte)\
  ((buf)->len < RINGBUF_SIZE ? \
    (buf)->data[RINGBUF_TAIL(buf)] = (byte), (sint32) ++(buf)->len : \
    (sint32) -1)
#define RINGBUF_GET(buf) \
  ((buf)->len > 0 ? \
    (buf)->len--, (sint32) (buf)->data[(buf)->head++ % RINGBUF_SIZE] : \
    (sint32) -1)


ringbuf_t uart0_rx_ringbuf;
ringbuf_t uart0_tx_ringbuf;

static uint16
uart0_rxfifo_move(ringbuf_t *dst, uint16 nbytes)
{
    uint16 i;

    for (i=0; i<nbytes; i++) {
        if (dst->len >= RINGBUF_SIZE) {
            break;
        }
        RINGBUF_PUT(dst, UART_RXFIFO_GET(UART0));
    }

    return i;
}

static uart_state state;

static void
uart0_intr_handler(void *arg)
{
    uint32 uart0_status;

    uart0_status = READ_PERI_REG(UART_INT_ST(UART0));
    if (uart0_status & UART_RXFIFO_TOUT_INT_ST) {
        uint16 uart0_rxfifo_len;

        // No character received for some time, read to ringbuf
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_ST);

        uart0_rxfifo_len = UART_RXFIFO_LEN(UART0);
        uart0_rxfifo_move(&uart0_rx_ringbuf, uart0_rxfifo_len);
    } else if (uart0_status & UART_RXFIFO_FULL_INT_ST) {
        uint16 uart0_rxfifo_len;

        // RX buffer becoming full, read to ringbuf
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_ST);

        uart0_rxfifo_len = UART_RXFIFO_LEN(UART0);
        uart0_rxfifo_move(&uart0_rx_ringbuf, uart0_rxfifo_len);
    } else if (uart0_status & UART_TXFIFO_EMPTY_INT_ST) {
        // TX buffer empty, check if ringbuf has space or disable int
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_TXFIFO_EMPTY_INT_ST);

        if (state.buf_sz == 0)
        {
            CLEAR_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);

            state.tx_done.up_isr();
            system_os_post(tos::esp82::main_task_prio, 0, 0);
            state.tx_buffer = nullptr;
        }
        else
        {
            auto c = *state.tx_buffer;
            ++state.tx_buffer;
            --state.buf_sz;
            WRITE_PERI_REG(UART_FIFO(UART0), c);
        }
    }
}

void ICACHE_FLASH_ATTR
uart0_open(uint32 baud_rate, uint32 flags)
{
    uint32 clkdiv;

    ETS_UART_INTR_DISABLE();

    // Set both RX and TX pins to correct mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);

    // Disable pullup on TX pin, enable on RX pin
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);

    // Configure baud rate for the port
    clkdiv = (UART_CLK_FREQ / baud_rate) & UART_CLKDIV_CNT;
    WRITE_PERI_REG(UART_CLKDIV(UART0), clkdiv);

    // Configure parameters for the port
    WRITE_PERI_REG(UART_CONF0(UART0), flags);

    // Reset UART0
    uart0_reset();
}

void ICACHE_FLASH_ATTR
uart0_reset()
{
    // Disable interrupts while resetting UART0
    ETS_UART_INTR_DISABLE();

    // Clear all RX and TX buffers and flags
    SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    RINGBUF_CLEAR(&uart0_rx_ringbuf);
    RINGBUF_CLEAR(&uart0_tx_ringbuf);

    // Set RX and TX interrupt thresholds
    WRITE_PERI_REG(UART_CONF1(UART0),
                   UART_RX_TOUT_EN |
                   ((UART_RXTOUT_TH & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S) |
                   ((UART_RXFIFO_TH & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |
                   ((UART_TXFIFO_TH & UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S));

    // Disable all existing interrupts and enable ours
    WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
    SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_RXFIFO_TOUT_INT_ENA);
    SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_RXFIFO_FULL_INT_ENA);

    // Restart the interrupt handler for UART0
    ETS_UART_INTR_ATTACH(uart0_intr_handler, NULL);
    ETS_UART_INTR_ENABLE();
}

uint16 ICACHE_FLASH_ATTR
uart0_available()
{
    uint16 ret;

    ETS_UART_INTR_DISABLE();
    ret = uart0_rx_ringbuf.len;
    ETS_UART_INTR_ENABLE();

    return ret;
}

uint16 ICACHE_FLASH_ATTR
uart0_read_buf(void *buf, uint16 nbytes, uint16 timeout)
{
    auto data = static_cast<uint8_t*>(buf);
    uint16 i;

    if (timeout > 0) {
        uint32 stime = system_get_time();

        // Wait until there is some data available
        while ((system_get_time() - stime) < ((uint32)timeout * 1000)) {
            if (uart0_available() > 0) break;
            os_delay_us(WAIT_RESOLUTION_US);
        }
    }

    // Read all data available in ringbuf
    ETS_UART_INTR_DISABLE();
    for (i=0; i<nbytes; i++) {
        if (uart0_rx_ringbuf.len == 0) break;
        data[i] = RINGBUF_GET(&uart0_rx_ringbuf);
    }
    ETS_UART_INTR_ENABLE();

    return i;
}

void uart0_tx_char_sync(uint8_t c)
{
    ETS_UART_INTR_DISABLE();

    while (true)
    {
        uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(UART0)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 1) {
            break;
        }
    }

    WRITE_PERI_REG(UART_FIFO(UART0), c);
}

void uart0_tx_buffer_sync(const uint8_t* buf, uint16_t len)
{
    while (len --> 0)
    {
        uart0_tx_char_sync(*buf++);
    }
}

void
uart0_tx_buffer(const uint8 *buf, uint16 len) {
    tos::lock_guard<tos::mutex> lk{state.tx_busy};

    ETS_UART_INTR_DISABLE();

    state.tx_buffer = (const char*)buf;
    state.buf_sz = len;

    SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
    ETS_UART_INTR_ENABLE();

    state.tx_done.down();
}

uint16 ICACHE_FLASH_ATTR
uart0_write_buf(const void *buf, uint16 nbytes, uint16 timeout)
{
    tos::lock_guard<tos::mutex> lk{state.tx_busy};

    uint32 stime;
    const uint8 *data = static_cast<const uint8_t*>(buf);
    uint16 i;

    if (timeout > 0) {
        uint32 stime = system_get_time();
        uint16 ringbuflen;

        // Wait until there is some space available
        while ((system_get_time() - stime) < ((uint32)timeout * 1000)) {
            ETS_UART_INTR_DISABLE();
            ringbuflen = uart0_tx_ringbuf.len;
            ETS_UART_INTR_ENABLE();
            if (ringbuflen < RINGBUF_SIZE) break;
            os_delay_us(WAIT_RESOLUTION_US);
        }
    }

    ETS_UART_INTR_DISABLE();
    for (i=0; i<nbytes; i++) {
        if (uart0_tx_ringbuf.len >= RINGBUF_SIZE) break;
        RINGBUF_PUT(&uart0_tx_ringbuf, data[i]);
    }
    SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
    ETS_UART_INTR_ENABLE();

    state.tx_done.down();

    return i;
}

void ICACHE_FLASH_ATTR
uart0_flush()
{
    while (UART_TXFIFO_LEN(UART0));
}

void ICACHE_FLASH_ATTR
uart1_open(uint32 baud_rate, uint32 flags)
{
    uint32 clkdiv;

    ETS_UART_INTR_DISABLE();

    // Set TX pin to correct mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);

    // Disable pullup on TX pin
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);

    // Configure baud rate for the port
    clkdiv = (UART_CLK_FREQ / baud_rate) & UART_CLKDIV_CNT;
    WRITE_PERI_REG(UART_CLKDIV(UART1), clkdiv);

    // Configure parameters for the port
    WRITE_PERI_REG(UART_CONF0(UART1), flags);

    // Reset UART1
    uart1_reset(baud_rate, flags);

    ETS_UART_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR
uart1_reset(uint32 baud_rate, uint32 flags)
{
    // Clear all TX buffers
    SET_PERI_REG_MASK(UART_CONF0(UART1), UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_TXFIFO_RST);
}

uint8 ICACHE_FLASH_ATTR
uart1_write_byte(uint8 byte)
{
    UART_TXFIFO_PUT(UART1, byte);
    while (UART_TXFIFO_LEN(UART1));
}
}
