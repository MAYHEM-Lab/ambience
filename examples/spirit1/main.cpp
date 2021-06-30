//
// Created by fatih on 6/10/19.
//

#include "common/spi.hpp"
#include "tos/stack_storage.hpp"

#include <MCU_Interface.h>
#include <SPIRIT_Config.h>
#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/sync_ring_buf.hpp>

namespace digital = tos::digital;
namespace pin_mode = tos::pin_mode;
namespace pin_change = tos::pin_change;
using namespace tos::stm32;
using namespace tos::tos_literals;
using namespace std::chrono_literals;

#define COMMAND_TX ((uint8_t)(0x60)) /*!< Start to transmit; valid only from READY */
#define COMMAND_RX ((uint8_t)(0x61)) /*!< Start to receive; valid only from READY */
#define COMMAND_READY                                                                    \
    ((uint8_t)(0x62)) /*!< Go to READY; valid only from STANDBY or SLEEP or LOCK */
#define COMMAND_STANDBY ((uint8_t)(0x63)) /*!< Go to STANDBY; valid only from READY */
#define COMMAND_SLEEP ((uint8_t)(0x64))   /*!< Go to SLEEP; valid only from READY */
#define COMMAND_LOCKRX                                                                   \
    ((uint8_t)(0x65)) /*!< Go to LOCK state by using the RX configuration of the synth;  \
                         valid only from READY */
#define COMMAND_LOCKTX                                                                   \
    ((uint8_t)(0x66)) /*!< Go to LOCK state by using the TX configuration of the synth;  \
                         valid only from READY */
#define COMMAND_SABORT                                                                   \
    ((uint8_t)(0x67)) /*!< Force exit form TX or RX states and go to READY state; valid  \
                         only from TX or RX */
#define COMMAND_SRES                                                                     \
    ((uint8_t)(0x70)) /*!< Reset of all digital part, except SPI registers */
#define COMMAND_FLUSHRXFIFO                                                              \
    ((uint8_t)(0x71)) /*!< Clean the RX FIFO; valid from all states */
#define COMMAND_FLUSHTXFIFO                                                              \
    ((uint8_t)(0x72)) /*!< Clean the TX FIFO; valid from all states */

#define HEADER_WRITE_MASK 0x00   /*!< Write mask for header byte*/
#define HEADER_READ_MASK 0x01    /*!< Read mask for header byte*/
#define HEADER_ADDRESS_MASK 0x00 /*!< Address mask for header byte*/
#define HEADER_COMMAND_MASK 0x80 /*!< Command mask for header byte*/

#define LINEAR_FIFO_ADDRESS 0xFF /*!< Linear FIFO address*/

#define BUILT_HEADER(add_comm, w_r)                                                      \
    (add_comm | w_r) /*!< macro to build the header                                      \
                        byte*/
#define WRITE_HEADER                                                                     \
    BUILT_HEADER(HEADER_ADDRESS_MASK,                                                    \
                 HEADER_WRITE_MASK) /*!< macro to build the write header byte*/
#define READ_HEADER                                                                      \
    BUILT_HEADER(HEADER_ADDRESS_MASK,                                                    \
                 HEADER_READ_MASK) /*!< macro to build the read header byte*/
#define COMMAND_HEADER                                                                   \
    BUILT_HEADER(HEADER_COMMAND_MASK,                                                    \
                 HEADER_WRITE_MASK) /*!< macro to build the command header byte*/

static gpio* gp;
static tos::stm32::spi* radio_spi;
auto cs_pin = 21_pin;
tos::sync_fixed_fifo<char, 128> pr;

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (tos::stm32::ahb_clock / 1'000'000)) / 13.3;
    for (volatile uint32_t i = 0; i < end; ++i) {
        __asm__ __volatile__("nop");
    }
};

StatusBytes to_stat(uint16_t stat) {
    StatusBytes s;
    memcpy(&s, &stat, sizeof s);
    return s;
}

extern "C" {
StatusBytes RadioSpiWriteRegisters(uint8_t address, uint8_t n_regs, uint8_t* buffer) {
    gp->write(cs_pin, digital::low);
    delay(5us);

    uint16_t msb = force_get(tos::spi::exchange(radio_spi, WRITE_HEADER));
    uint16_t lsb = force_get(tos::spi::exchange(radio_spi, address));


    auto tmpstatus = msb << 8 | lsb;
    for (int i = 0; i < n_regs; i++) {
        force_get(tos::spi::exchange(radio_spi, buffer[i]));
    }

    gp->write(cs_pin, digital::high);

    return to_stat(tmpstatus);
}

StatusBytes RadioSpiReadRegisters(uint8_t address, uint8_t n_regs, uint8_t* buffer) {
    gp->write(cs_pin, digital::low);
    delay(5us);

    uint16_t msb = force_get(tos::spi::exchange(radio_spi, READ_HEADER));
    uint16_t lsb = force_get(tos::spi::exchange(radio_spi, address));

    auto tmpstatus = msb << 8 | lsb;

    for (int i = 0; i < n_regs; i++)
        buffer[i] = force_get(tos::spi::exchange(radio_spi, 0));

    gp->write(cs_pin, digital::high);

    return to_stat(tmpstatus);
}

StatusBytes RadioSpiWriteFifo(uint8_t n_regs, uint8_t* buffer) {
    static uint16_t tmpstatus;

    StatusBytes* status = (StatusBytes*)&tmpstatus;

    gp->write(cs_pin, digital::low);
    delay(5us);

    tmpstatus = (uint16_t)(force_get(tos::spi::exchange(radio_spi, WRITE_HEADER)) << 8 |
                           force_get(tos::spi::exchange(radio_spi, LINEAR_FIFO_ADDRESS)));

    for (int i = 0; i < n_regs; i++) {
        force_get(tos::spi::exchange(radio_spi, buffer[i]));
    }

    gp->write(cs_pin, digital::high);

    return *status;
}

StatusBytes RadioSpiReadFifo(uint8_t n_regs, uint8_t* buffer) {
    static uint16_t tmpstatus;

    StatusBytes* status = (StatusBytes*)&tmpstatus;

    gp->write(cs_pin, digital::low);
    delay(5us);

    tmpstatus = (uint16_t)(force_get(tos::spi::exchange(radio_spi, READ_HEADER)) << 8 |
                           force_get(tos::spi::exchange(radio_spi, LINEAR_FIFO_ADDRESS)));

    for (int i = 0; i < n_regs; i++)
        buffer[i] = force_get(tos::spi::exchange(radio_spi, 0));

    gp->write(cs_pin, digital::high);

    return *status;
}

StatusBytes RadioSpiCommandStrobes(uint8_t cmd_code) {
    gp->write(cs_pin, digital::low);
    delay(5us);

    uint16_t msb = force_get(tos::spi::exchange(radio_spi, COMMAND_HEADER));
    uint16_t lsb = force_get(tos::spi::exchange(radio_spi, cmd_code));

    auto tmpstatus = msb << 8 | lsb;

    gp->write(cs_pin, digital::high);

    return to_stat(tmpstatus);
}

/* This is the function that initializes the SPIRIT with the configuration
that the user has exported using the GUI */
void SpiritBaseConfiguration(void) {
    uint8_t tmp[7];

    /* Be sure that the registers config is default */
    SpiritSpiCommandStrobes(COMMAND_SRES);

    /* Extra current in after power on fix.
       In some samples, when a supply voltage below 2.6 V is applied to SPIRIT1 from a no
       power condition, an extra current is added to the typical current consumption. With
       this sequence, the extra current is erased.
    */
    tmp[0] = 0xCA;
    SpiritSpiWriteRegisters(0xB2, 1, tmp);
    tmp[0] = 0x04;
    SpiritSpiWriteRegisters(0xA8, 1, tmp);
    SpiritSpiReadRegisters(0xA8, 1, tmp);
    tmp[0] = 0x00;
    SpiritSpiWriteRegisters(0xA8, 1, tmp);

    tmp[0] = 0xA3; /* reg. GPIO3_CONF (0x02) */
    SpiritSpiWriteRegisters(0x02, 1, tmp);
    tmp[0] = 0x36; /* reg. IF_OFFSET_ANA (0x07) */
    tmp[1] = 0x06; /* reg. SYNT3 (0x08) */
    tmp[2] = 0x82; /* reg. SYNT2 (0x09) */
    tmp[3] = 0x54; /* reg. SYNT1 (0x0A) */
    tmp[4] = 0x61; /* reg. SYNT0 (0x0B) */
    tmp[5] = 0x01; /* reg. CH_SPACE (0x0C) */
    tmp[6] = 0xAC; /* reg. IF_OFFSET_DIG (0x0D) */
    SpiritSpiWriteRegisters(0x07, 7, tmp);
    tmp[0] = 0x17; /* reg. PA_POWER[8] (0x10) */
    SpiritSpiWriteRegisters(0x10, 1, tmp);
    tmp[0] = 0x93; /* reg. MOD1 (0x1A) */
    tmp[1] = 0x09; /* reg. MOD0 (0x1B) */
    tmp[2] = 0x41; /* reg. FDEV0 (0x1C) */
    tmp[3] = 0x13; /* reg. CHFLT (0x1D) */
    tmp[4] = 0xC8; /* reg. AFC2 (0x1E) */
    SpiritSpiWriteRegisters(0x1A, 5, tmp);
    tmp[0] = 0x64; /* reg. RSSI_TH (0x22) */
    SpiritSpiWriteRegisters(0x22, 1, tmp);
    tmp[0] = 0x62; /* reg. AGCCTRL1 (0x25) */
    SpiritSpiWriteRegisters(0x25, 1, tmp);
    tmp[0] = 0x15; /* reg. ANT_SELECT_CONF (0x27) */
    SpiritSpiWriteRegisters(0x27, 1, tmp);
    tmp[0] = 0x1F; /* reg. PCKTCTRL2 (0x32) */
    tmp[1] = 0x40; /* reg. PCKTCTRL1 (0x33) */
    SpiritSpiWriteRegisters(0x32, 2, tmp);
    tmp[0] = 0x12; /* reg. PCKTLEN0 (0x35) */
    tmp[1] = 0x91; /* reg. SYNC4 (0x36) */
    tmp[2] = 0xD3; /* reg. SYNC3 (0x37) */
    tmp[3] = 0x91; /* reg. SYNC2 (0x38) */
    tmp[4] = 0xD3; /* reg. SYNC1 (0x39) */
    SpiritSpiWriteRegisters(0x35, 5, tmp);
    tmp[0] = 0x41; /* reg. PCKT_FLT_OPTIONS (0x4F) */
    tmp[1] = 0x40; /* reg. PROTOCOL[2] (0x50) */
    tmp[2] = 0x01; /* reg. PROTOCOL[1] (0x51) */
    SpiritSpiWriteRegisters(0x4F, 3, tmp);
    tmp[0] = 0x09; /* reg. TIMERS[5] (0x53) */
    SpiritSpiWriteRegisters(0x53, 1, tmp);
    tmp[0] = 0x3E; /* reg. RCO_VCO_CALIBR_IN[1] (0x6E) */
    tmp[1] = 0x3F; /* reg. RCO_VCO_CALIBR_IN[0] (0x6F) */
    SpiritSpiWriteRegisters(0x6E, 2, tmp);
    tmp[0] = 0x02; /* reg. IRQ_MASK[1] (0x92) */
    SpiritSpiWriteRegisters(0x92, 1, tmp);
    tmp[0] = 0xA0; /* reg. SYNTH_CONFIG[0] (0x9F) */
    SpiritSpiWriteRegisters(0x9F, 1, tmp);
    tmp[0] = 0x25; /* reg. VCO_CONFIG (0xA1) */
    SpiritSpiWriteRegisters(0xA1, 1, tmp);
    tmp[0] = 0x35; /* reg. DEM_CONFIG (0xA3) */
    SpiritSpiWriteRegisters(0xA3, 1, tmp);
    tmp[0] = 0x98; /* reg. PM_CONFIG[1] (0xA5) */
    SpiritSpiWriteRegisters(0xA5, 1, tmp);

    /* VCO unwanted calibration workaround.
       With this sequence, the PA is on after the eventual VCO calibration expires.
    */
    tmp[0] = 0x22;
    SpiritSpiWriteRegisters(0xBC, 1, tmp);
}

/* This is a VCO calibration routine used to recalibrate the VCO of SPIRIT1 in a safe way.
 IMPORTANT: It must be called from READY state. */
void SpiritVcoCalibration(void) {
    uint8_t tmp[4];
    uint8_t cal_words[2];
    uint8_t state;

    SpiritSpiReadRegisters(0x9E, 1, tmp);
    tmp[0] |= 0x80;
    SpiritSpiWriteRegisters(0x9E, 1, tmp); /* REFDIV bit set (to be restored) */

    /* As a consequence we need to double the SYNT word to generate the target frequency
     */
    tmp[0] = 0x0D;
    tmp[1] = 0x04;
    tmp[2] = 0xA8;
    tmp[3] = 0xC1;
    SpiritSpiWriteRegisters(0x08, 4, tmp);


    tmp[0] = 0x25;
    SpiritSpiWriteRegisters(0xA1, 1, tmp); /* increase VCO current (restore to 0x11) */

    SpiritSpiReadRegisters(0x50, 1, tmp);
    tmp[0] |= 0x02;
    SpiritSpiWriteRegisters(0x50, 1, tmp); /* enable VCO calibration (to be restored) */

    SpiritSpiCommandStrobes(COMMAND_LOCKTX);
    do {
        SpiritSpiReadRegisters(0xC1, 1, &state);
    } while ((state & 0xFE) != 0x1E); /* wait until LOCK (MC_STATE = 0x0F <<1) */
    SpiritSpiReadRegisters(0xE5, 1, &cal_words[0]); /* calib out word for TX */

    SpiritSpiCommandStrobes(COMMAND_READY);
    do {
        SpiritSpiReadRegisters(0xC1, 1, &state);
    } while ((state & 0xFE) != 0x06); /* wait until READY (MC_STATE = 0x03 <<1) */

    SpiritSpiCommandStrobes(COMMAND_LOCKRX);
    do {
        SpiritSpiReadRegisters(0xC1, 1, &state);
    } while ((state & 0xFE) != 0x1E); /* wait until LOCK (MC_STATE = 0x0F <<1) */
    SpiritSpiReadRegisters(0xE5, 1, &cal_words[1]); /* calib out word for RX */

    SpiritSpiCommandStrobes(COMMAND_READY);
    do {
        SpiritSpiReadRegisters(0xC1, 1, &state);
    } while ((state & 0xFE) != 0x06); /* wait until READY (MC_STATE = 0x03 <<1) */

    SpiritSpiReadRegisters(0x50, 1, tmp);
    tmp[0] &= 0xFD;
    SpiritSpiWriteRegisters(0x50, 1, tmp); /* VCO calib restored to 0 */

    SpiritSpiReadRegisters(0x9E, 1, tmp);
    tmp[0] &= 0x7F;
    SpiritSpiWriteRegisters(0x9E, 1, tmp); /* REFDIV bit reset */


    tmp[0] = 0x06;
    tmp[1] = 0x82;
    tmp[2] = 0x54;
    tmp[3] = 0x61;
    SpiritSpiWriteRegisters(0x08, 4, tmp); /* SYNTH WORD restored */


    SpiritSpiWriteRegisters(0x6E, 2, cal_words); /* write both calibration words */
}
}

auto print_task = [](auto&& usart) {
    while (true) {
        auto c = pr.pop();
        tos::println(usart, int(c));
    }
};

tos::stm32::exti external_interrupts;
tos::semaphore interrupt{0};

auto tx = []() {
    SpiritIrq(TX_DATA_SENT, S_ENABLE);

    SpiritIrqClearStatus();

    SpiritCmdStrobeFlushTxFifo();
    SpiritSpiWriteLinearFifo(5, (uint8_t*)"HELLO");

    /* send the TX command */
    SpiritManagementWaCmdStrobeTx();
    SpiritCmdStrobeTx();

    // while(g->read(exti_pin));

    SpiritIrqs xIrqStatus;
    do {
        interrupt.down();
        SpiritIrqGetStatus(&xIrqStatus);
        SpiritIrqClearStatus();
        if (!xIrqStatus.IRQ_TX_DATA_SENT)
            continue;
        break;
    } while (true);

    return xIrqStatus;
};

auto rx = []() {
    SpiritIrq(RX_DATA_DISC, S_ENABLE);
    SpiritIrq(RX_DATA_READY, S_ENABLE);

    /* enable SQI check */
    SpiritQiSetSqiThreshold(SQI_TH_0);
    SpiritQiSqiCheck(S_ENABLE);

    /* RX timeout config */
    SpiritTimerSetRxTimeoutMs(2000.0);
    SpiritTimerSetRxTimeoutStopCondition(SQI_ABOVE_THRESHOLD);

    SpiritCmdStrobeRx();

    interrupt.down();

    SpiritIrqs xIrqStatus;
    SpiritIrqGetStatus(&xIrqStatus);
    SpiritIrqClearStatus();

    return xIrqStatus;
};

void radio_task(bool is_tx) {
    auto sdn_pin = 31_pin;
    auto cs_pin = 21_pin;
    auto exti_pin = 69_pin;

    auto sck_pin = 42_pin;
    auto miso_pin = 43_pin;
    auto mosi_pin = 44_pin;

    auto usart_rx_pin = 23_pin;
    auto usart_tx_pin = 22_pin;

    auto g = tos::open(tos::devs::gpio);

    g->set_pin_mode(sdn_pin, tos::pin_mode::out);
    g->write(sdn_pin, tos::digital::high); // active low

    g->set_pin_mode(cs_pin, tos::pin_mode::out);
    g->write(cs_pin, tos::digital::high);

    g->set_pin_mode(exti_pin, pin_mode::in_pullup);

    auto timer = open(tos::devs::timer<2>);
    tos::alarm alarm(&timer);

    auto usart = tos::open(
        tos::devs::usart<1>, tos::uart::default_9600, usart_rx_pin, usart_tx_pin);
    tos::println(usart, "hello");

    tos::stm32::spi s(tos::stm32::detail::spis[2], sck_pin, miso_pin, mosi_pin);

    using namespace std::chrono_literals;
    g->write(sdn_pin, tos::digital::high); // wake up
    tos::this_thread::sleep_for(alarm, 5ms);

    g->write(sdn_pin, tos::digital::low); // wake up
    tos::this_thread::sleep_for(alarm, 10ms);

    radio_spi = &s;
    gp = &g;

    auto res = SpiritSpiCommandStrobes(COMMAND_SRES);
    tos::println(usart, int(res.MC_STATE));

    auto pn = SpiritGeneralGetSpiritVersion();
    tos::println(usart, int(pn));

    SpiritManagementWaExtraCurrent();

    /* Manually set the XTAL_FREQUENCY */
    SpiritRadioSetXtalFrequency(52000000);

    SpiritBaseConfiguration();

    SRadioInit xradio;
    SpiritRadioGetInfo(&xradio);

    tos::println(usart, "datarate", int(xradio.lDatarate));
    tos::println(usart, "channel", int(xradio.nChannelSpace), int(xradio.cChannelNumber));
    tos::println(usart, "bw", int(xradio.lBandwidth));

    SGpioInit xGpioIRQ;
    xGpioIRQ.xSpiritGpioPin = SPIRIT_GPIO_3;
    xGpioIRQ.xSpiritGpioMode = SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP;
    xGpioIRQ.xSpiritGpioIO = SPIRIT_GPIO_DIG_OUT_IRQ;

    SpiritGpioInit(&xGpioIRQ);

    if (is_tx) {
        SpiritRadioSetPALeveldBm(7, 5);
        SpiritRadioSetPALevelMaxIndex(7);
        SpiritPktCommonSetMyAddress(0xAB);
        SpiritPktCommonSetDestinationAddress(0xBA);
    } else {
        SpiritPktCommonSetMyAddress(0xBA);
    }

    SpiritIrqClearStatus();
    SpiritIrqDeInit(nullptr);
    tos::this_thread::sleep_for(alarm, 1s);

    auto ext_handler = [] { interrupt.up_isr(); };
    external_interrupts.attach(
        5_pin, pin_change::falling, tos::function_ref<void()>(ext_handler));

    reset(interrupt, 0);

    while (true) {
        tos::println(usart, "clear");
        SpiritIrqClearStatus();
        tos::println(usart, "deinit");
        SpiritIrqDeInit(nullptr);
        tos::println(usart, "mm");
        if (is_tx) {
            tos::println(usart, "call tx");

            auto xIrqStatus = tx();

            tos::println(usart, "tx:", bool(xIrqStatus.IRQ_TX_DATA_SENT));
            tos::this_thread::sleep_for(alarm, 1000ms);
            tos::println(usart, "woke");
        } else {
            auto xIrqStatus = rx();
            tos::println(usart,
                         "rx:",
                         bool(xIrqStatus.IRQ_RX_DATA_READY),
                         bool(xIrqStatus.IRQ_RX_DATA_DISC));

            if (xIrqStatus.IRQ_RX_DATA_DISC) {
                SpiritCmdStrobeFlushRxFifo();
            }
            if (!xIrqStatus.IRQ_RX_DATA_READY) {
                continue;
            }

            auto cRxData = SpiritLinearFifoReadNumElementsRxFifo();
            tos::println(usart, "rx:", cRxData);

            /* Read the RX FIFO */
            std::vector<char> buff(cRxData);
            SpiritSpiReadLinearFifo(cRxData, (uint8_t*)buff.data());
            tos::println(usart, "rx:", buff);

            /* Flush the RX FIFO */
            SpiritCmdStrobeFlushRxFifo();
        }
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, radio_task, false);
}