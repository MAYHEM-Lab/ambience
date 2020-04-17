#include <arch/usart.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
extern "C" {
#include "ets_sys.h"
#include "new_uart_priv.h"
#include "osapi.h"
#include "uart_register.h"

#include <lx106_missing.hpp>
#include <user_interface.h>
}

namespace {
struct UART_CONTROL_BLOCK {
    union {
        struct {
            uint8_t rw_byte; /*This register stores one byte data  read by rx fifo.*/
            uint8_t reserved[3];
        };
        const uint32_t val;
    } fifo;
    union {
        struct {
            uint32_t rxfifo_full : 1;  /*This interrupt raw bit turns to high level when
                                          receiver receives more data than
                                          (rx_flow_thrhd_h3 rx_flow_thrhd).*/
            uint32_t txfifo_empty : 1; /*This interrupt raw bit turns to high level when
                                          the amount of data in transmitter's fifo is less
                                          than ((tx_mem_cnttxfifo_cnt) .*/
            uint32_t parity_err : 1;   /*This interrupt raw bit turns to high level when
                                          receiver detects the parity error of data.*/
            uint32_t frm_err : 1;      /*This interrupt raw bit turns to high level when
                                          receiver detects data's frame error .*/
            uint32_t
                rxfifo_ovf : 1; /*This interrupt raw bit turns to high level when receiver
                                   receives more data than the fifo can store.*/
            uint32_t dsr_chg : 1;     /*This interrupt raw bit turns to high level when
                                         receiver detects the edge change of dsrn signal.*/
            uint32_t cts_chg : 1;     /*This interrupt raw bit turns to high level when
                                         receiver detects the edge change of ctsn signal.*/
            uint32_t brk_det : 1;     /*This interrupt raw bit turns to high level when
                                         receiver detects the 0 after the stop bit.*/
            uint32_t rxfifo_tout : 1; /*This interrupt raw bit turns to high level when
                                         receiver takes more time than rx_tout_thrhd to
                                         receive a byte.*/
            uint32_t reserved9 : 23;
        };
        const uint32_t val;
    } int_raw;
    union {
        struct {
            uint32_t rxfifo_full : 1;  /*This is the status bit for rxfifo_full_int_raw
                                          when rxfifo_full_int_ena is set to 1.*/
            uint32_t txfifo_empty : 1; /*This is the status bit for  txfifo_empty_int_raw
                                          when txfifo_empty_int_ena is set to 1.*/
            uint32_t parity_err : 1;  /*This is the status bit for parity_err_int_raw when
                                         parity_err_int_ena is set to 1.*/
            uint32_t frm_err : 1;     /*This is the status bit for frm_err_int_raw when
                                         fm_err_int_ena is set to 1.*/
            uint32_t rxfifo_ovf : 1;  /*This is the status bit for rxfifo_ovf_int_raw when
                                         rxfifo_ovf_int_ena is set to 1.*/
            uint32_t dsr_chg : 1;     /*This is the status bit for dsr_chg_int_raw when
                                         dsr_chg_int_ena is set to 1.*/
            uint32_t cts_chg : 1;     /*This is the status bit for cts_chg_int_raw when
                                         cts_chg_int_ena is set to 1.*/
            uint32_t brk_det : 1;     /*This is the status bit for brk_det_int_raw when
                                         brk_det_int_ena is set to 1.*/
            uint32_t rxfifo_tout : 1; /*This is the status bit for rxfifo_tout_int_raw
                                         when rxfifo_tout_int_ena is set to 1.*/
            uint32_t reserved9 : 23;
        };
        const uint32_t val;
    } int_st;
    union {
        struct {
            uint32_t rxfifo_full : 1;  /*This is the enable bit for rxfifo_full_int_st
                                          register.*/
            uint32_t txfifo_empty : 1; /*This is the enable bit for rxfifo_full_int_st
                                          register.*/
            uint32_t
                parity_err : 1; /*This is the enable bit for parity_err_int_st register.*/
            uint32_t frm_err : 1; /*This is the enable bit for frm_err_int_st register.*/
            uint32_t
                rxfifo_ovf : 1; /*This is the enable bit for rxfifo_ovf_int_st register.*/
            uint32_t dsr_chg : 1; /*This is the enable bit for dsr_chg_int_st register.*/
            uint32_t cts_chg : 1; /*This is the enable bit for cts_chg_int_st register.*/
            uint32_t brk_det : 1; /*This is the enable bit for brk_det_int_st register.*/
            uint32_t rxfifo_tout : 1; /*This is the enable bit for rxfifo_tout_int_st
                                         register.*/
            uint32_t reserved9 : 23;
        };
        uint32_t val;
    } int_ena;
    union {
        struct {
            uint32_t rxfifo_full : 1;  /*Set this bit to clear the rxfifo_full_int_raw
                                          interrupt.*/
            uint32_t txfifo_empty : 1; /*Set this bit to clear txfifo_empty_int_raw
                                          interrupt.*/
            uint32_t
                parity_err : 1;   /*Set this bit to clear parity_err_int_raw interrupt.*/
            uint32_t frm_err : 1; /*Set this bit to clear frm_err_int_raw interrupt.*/
            uint32_t
                rxfifo_ovf : 1;   /*Set this bit to clear rxfifo_ovf_int_raw interrupt.*/
            uint32_t dsr_chg : 1; /*Set this bit to clear the dsr_chg_int_raw interrupt.*/
            uint32_t cts_chg : 1; /*Set this bit to clear the cts_chg_int_raw interrupt.*/
            uint32_t brk_det : 1; /*Set this bit to clear the brk_det_int_raw interrupt.*/
            uint32_t rxfifo_tout : 1; /*Set this bit to clear the rxfifo_tout_int_raw
                                         interrupt.*/
            uint32_t reserved9 : 23;
        };
        uint32_t val;
    } int_clr;
    union {
        struct {
            uint32_t div_int : 20; /*The register value is  the  integer part of the
                                      frequency divider's factor.*/
            uint32_t reserved20 : 12;
        };
        uint32_t val;
    } clk_div;
    union {
        struct {
            uint32_t en : 1; /*This is the enable bit for detecting baudrate.*/
            uint32_t reserved1 : 7;
            uint32_t glitch_filt : 8; /*when input pulse width is lower then this value
                                         ignore this pulse.this register is used in
                                         auto-baud detect process.*/
            uint32_t reserved16 : 16;
        };
        uint32_t val;
    } auto_baud;
    union {
        struct {
            uint32_t rxfifo_cnt : 8; /*(rx_mem_cnt rxfifo_cnt) stores the byte number of
                                        valid data in receiver's fifo. rx_mem_cnt register
                                        stores the 3 most significant bits  rxfifo_cnt
                                        stores the 8 least significant bits.*/
            uint32_t reserved8 : 5;
            uint32_t dsrn : 1; /*This register stores the level value of the internal uart
                                  dsr signal.*/
            uint32_t ctsn : 1; /*This register stores the level value of the internal uart
                                  cts signal.*/
            uint32_t rxd : 1;  /*This register stores the level value of the internal uart
                                  rxd signal.*/
            uint32_t txfifo_cnt : 8; /*(tx_mem_cnt txfifo_cnt) stores the byte number of
                                        valid data in transmitter's fifo.tx_mem_cnt stores
                                        the 3 most significant bits  txfifo_cnt stores the
                                        8 least significant bits.*/
            uint32_t reserved24 : 5;
            uint32_t dtrn : 1; /*The register represent the level value of the internal
                                  uart dsr signal.*/
            uint32_t rtsn : 1; /*This register represent the level value of the internal
                                  uart cts signal.*/
            uint32_t txd : 1;  /*This register represent the  level value of the internal
                                  uart rxd signal.*/
        };
        const uint32_t val;
    } status;
    union {
        struct {
            uint32_t parity : 1;    /*This register is used to configure the parity check
                                       mode.  0:even 1:odd*/
            uint32_t parity_en : 1; /*Set this bit to enable uart parity check.*/
            uint32_t bit_num : 2;   /*This register is used to set the length of data:
                                       0:5bits 1:6bits 2:7bits 3:8bits*/
            uint32_t stop_bit_num : 2; /*This register is used to set the length of  stop
                                          bit. 1:1bit  2:1.5bits  3:2bits*/
            uint32_t sw_rts : 1;  /*This register is used to configure the software rts
                                     signal which is used in software flow control.*/
            uint32_t sw_dtr : 1;  /*This register is used to configure the software dtr
                                     signal which is used in software flow control..*/
            uint32_t txd_brk : 1; /*Set this bit to enable transmitter to  send 0 when the
                                     process of sending data is done.*/
            uint32_t irda_dplx : 1; /*Set this bit to enable irda loop-back mode.*/
            uint32_t
                irda_tx_en : 1; /*This is the start enable bit for irda transmitter.*/
            uint32_t
                irda_wctl : 1; /*1：the irda transmitter's 11th bit is the same to the
                                  10th bit. 0：set irda transmitter's 11th bit to 0.*/
            uint32_t irda_tx_inv : 1; /*Set this bit to inverse the level value of  irda
                                         transmitter's level.*/
            uint32_t irda_rx_inv : 1; /*Set this bit to inverse the level value of irda
                                         receiver's level.*/
            uint32_t loopback : 1;    /*Set this bit to enable uart loop-back test mode.*/
            uint32_t tx_flow_en : 1;  /*Set this bit to enable transmitter's flow control
                                         function.*/
            uint32_t irda_en : 1;     /*Set this bit to enable irda protocol.*/
            uint32_t rxfifo_rst : 1;  /*Set this bit to reset uart receiver's fifo.*/
            uint32_t txfifo_rst : 1;  /*Set this bit to reset uart transmitter's fifo.*/
            uint32_t rxd_inv : 1; /*Set this bit to inverse the level value of uart rxd
                                     signal.*/
            uint32_t cts_inv : 1; /*Set this bit to inverse the level value of uart cts
                                     signal.*/
            uint32_t dsr_inv : 1; /*Set this bit to inverse the level value of uart dsr
                                     signal.*/
            uint32_t txd_inv : 1; /*Set this bit to inverse the level value of uart txd
                                     signal.*/
            uint32_t rts_inv : 1; /*Set this bit to inverse the level value of uart rts
                                     signal.*/
            uint32_t dtr_inv : 1; /*Set this bit to inverse the level value of uart dtr
                                     signal.*/
            uint32_t reserved25 : 7;
        };
        uint32_t val;
    } conf0;
    union {
        struct {
            uint32_t
                rxfifo_full_thrhd : 7; /*When receiver receives more data than its
                                          threshold value，receiver will produce
                                          rxfifo_full_int_raw interrupt.the threshold
                                          value is (rx_flow_thrhd_h3 rxfifo_full_thrhd).*/
            uint32_t reserved7 : 1;
            uint32_t txfifo_empty_thrhd : 7; /*when the data amount in transmitter fifo is
                                                less than its threshold value， it will
                                                produce txfifo_empty_int_raw interrupt.
                                                the threshold value is
                                                (tx_mem_empty_thresh txfifo_empty_thrhd)*/
            uint32_t reserved15 : 1;
            uint32_t
                rx_flow_thrhd : 7; /*when receiver receives more data than its threshold
                                      value， receiver produce signal to tell the
                                      transmitter stop transferring data. the threshold
                                      value is (rx_flow_thrhd_h3 rx_flow_thrhd).*/
            uint32_t
                rx_flow_en : 1; /*This is the flow enable bit for uart receiver. 1:choose
                                   software flow control with configuring sw_rts signal*/
            uint32_t rx_tout_thrhd : 7; /*This register is used to configure the timeout
                                           value for uart receiver receiving a byte.*/
            uint32_t rx_tout_en : 1; /*This is the enable bit for uart receiver's timeout
                                        function.*/
        };
        uint32_t val;
    } conf1;
    union {
        struct {
            uint32_t min_cnt : 20; /*This register stores the value of the minimum
                                      duration time for the low level pulse， it is used
                                      in baudrate-detect process.*/
            uint32_t reserved20 : 12;
        };
        const uint32_t val;
    } lowpulse;
    union {
        struct {
            uint32_t min_cnt : 20; /*This register stores  the value of the maximum
                                      duration time for the high level pulse， it is used
                                      in baudrate-detect process.*/
            uint32_t reserved20 : 12;
        };
        const uint32_t val;
    } highpulse;
    union {
        struct {
            uint32_t edge_cnt : 10; /*This register stores the count of rxd edge change，
                                       it is used in baudrate-detect process.*/
            uint32_t reserved10 : 22;
        };
        const uint32_t val;
    } rxd_cnt;
    uint32_t reserved[18];
    uint32_t date; /**/
    uint32_t id;   /**/
};

void tx_char_sync(volatile UART_CONTROL_BLOCK* uart, uint8_t byte) {
    while (uart->status.txfifo_cnt != 0)
        ;
    uart->fifo.rw_byte = byte;
}

void tx_buffer_sync(volatile UART_CONTROL_BLOCK* uart, tos::span<const uint8_t> buffer) {
    for (auto& byte : buffer) {
        tx_char_sync(uart, byte);
    }
}

struct uart_state {
    tos::span<const uint8_t> tx_buffer{nullptr};
    tos::basic_fixed_fifo<uint8_t, 128, tos::ring_buf> rx_buf;
    tos::semaphore rx_count{0};
    tos::semaphore tx_done{0};
    tos::mutex tx_busy;
};

uart_state state;

void interrupt_handler(void* arg) {
    auto uart = static_cast<volatile UART_CONTROL_BLOCK*>(arg);
    auto& status = uart->int_st;

    if (status.rxfifo_full || status.rxfifo_tout) {
        auto uart0_rxfifo_len = uart->status.rxfifo_cnt;
        tos::debug::log("Have", uart0_rxfifo_len, "bytes");
        for (size_t i = 0; i < uart0_rxfifo_len; ++i) {
            if (state.rx_buf.size() == state.rx_buf.capacity()) {
                tos::debug::do_not_optimize(uart->fifo.rw_byte);
                break;
            }
            uint8_t byte = uart->fifo.rw_byte;
            state.rx_buf.push(byte);
            state.rx_count.up_isr();
        }
        system_os_post(tos::esp82::main_task_prio, 0, 0);
        uart->int_clr.rxfifo_full = 1;
        uart->int_clr.rxfifo_tout = 1;
    }

    if (status.txfifo_empty) {
        if (state.tx_buffer.empty()) {
            uart->int_ena.txfifo_empty = false;

            state.tx_done.up_isr();
            system_os_post(tos::esp82::main_task_prio, 0, 0);
            state.tx_buffer = tos::span<const uint8_t>{nullptr};
        } else {
            uart->fifo.rw_byte = state.tx_buffer.front();
            state.tx_buffer = state.tx_buffer.slice(1);
        }
        uart->int_clr.txfifo_empty = 1;
    }
}
}

extern "C" {
// Missing defines from SDK
#define FUNC_U0RXD 0
auto UART0 = reinterpret_cast<volatile UART_CONTROL_BLOCK*>(REG_UART_BASE(0));
auto UART1 = reinterpret_cast<volatile UART_CONTROL_BLOCK*>(REG_UART_BASE(1));

ICACHE_FLASH_ATTR
void uart0_open(uint32 baud_rate, uint32 flags) {
    uint32 clkdiv;

    ETS_UART_INTR_DISABLE();

    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
    // Set both RX and TX pins to correct mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);

    // Configure baud rate for the port
    clkdiv = (UART_CLK_FREQ / baud_rate) & UART_CLKDIV_CNT;
    UART0->clk_div.div_int = clkdiv;

    // Configure parameters for the port
    UART0->conf0.parity_en = 0;
    UART0->conf0.stop_bit_num = 1;
    UART0->conf0.bit_num = 3;

    // Reset UART0
    uart0_reset();
}

ICACHE_FLASH_ATTR
void uart0_reset() {
    // Disable interrupts while resetting UART0
    ETS_UART_INTR_DISABLE();

    // Clear all RX and TX buffers and flags
    UART0->conf0.val |= UART_RXFIFO_RST | UART_TXFIFO_RST;
    UART0->conf0.val &= ~(UART_RXFIFO_RST | UART_TXFIFO_RST);

    // Set RX and TX interrupt thresholds
    UART0->conf1.rx_tout_en = 1;
    UART0->conf1.rx_tout_thrhd = 10;
    UART0->conf1.rxfifo_full_thrhd = 100;
    UART0->conf1.txfifo_empty_thrhd = 1;

    // Disable all existing interrupts and enable ours
    UART0->int_clr.val = 0xFFFF;
    UART0->int_ena.val |= UART_RXFIFO_TOUT_INT_ENA | UART_RXFIFO_FULL_INT_ENA;

    // Restart the interrupt handler for UART0
    ETS_UART_INTR_ATTACH(interrupt_handler, UART0);
    ETS_UART_INTR_ENABLE();
}

ICACHE_FLASH_ATTR
void uart1_open(uint32 baud_rate, uint32 flags) {
    uint32 clkdiv;

    ETS_UART_INTR_DISABLE();

    // Set TX pin to correct mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);

    // Disable pullup on TX pin
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);

    // Configure baud rate for the port
    clkdiv = (UART_CLK_FREQ / baud_rate) & UART_CLKDIV_CNT;
    UART1->clk_div.div_int = clkdiv;

    // Configure parameters for the port
    UART1->conf0.val = flags;

    // Reset UART1
    uart1_reset(baud_rate, flags);

    ETS_UART_INTR_ENABLE();
}

ICACHE_FLASH_ATTR
void uart1_reset(uint32 baud_rate, uint32 flags) {
    // Clear all TX buffers
    UART1->conf0.txfifo_rst = 1;
    UART1->conf0.txfifo_rst = 0;
}
}

namespace tos::esp82 {
ICACHE_FLASH_ATTR
span<uint8_t> uart0::read(span<uint8_t> buf) {
    for (auto& byte : buf) {
        state.rx_count.down();
        byte = state.rx_buf.pop();
    }
    return buf;
}

ICACHE_FLASH_ATTR
uart0::uart0(usart_constraint params) {
    ::uart0_open(get<usart_baud_rate>(params).rate, 0);
}

int ICACHE_FLASH_ATTR uart0::write(tos::span<const uint8_t> buf) {
    tos::lock_guard<tos::mutex> lk{state.tx_busy};

    ETS_UART_INTR_DISABLE();

    state.tx_buffer = buf;

    UART0->int_ena.txfifo_empty = 1;
    ETS_UART_INTR_ENABLE();

    state.tx_done.down();
    return buf.size();
}

void uart0::isr() {
}

ICACHE_FLASH_ATTR
int sync_uart0::write(span<const uint8_t> buf) {
    tx_buffer_sync(UART0, buf);
    return buf.size();
}
} // namespace tos::esp82