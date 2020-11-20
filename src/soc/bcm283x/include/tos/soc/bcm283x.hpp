#pragma once

#include <cstdint>
#include <cstddef>

namespace tos::bcm283x {
struct uart0_control_block {
    uint32_t DR;
    uint32_t RSRECR;
    uint8_t __pad__[16];
    uint32_t FR;
    uint8_t __pad_[4];
    uint32_t ILPR;
    uint32_t IBRD;
    uint32_t FBRD;
    uint32_t LCRH;
    uint32_t CR;
    uint32_t IFLS;
    uint32_t IMSC;
    uint32_t RIS;
    uint32_t MIS;
    uint32_t ICR;
    uint32_t DMACR;
    uint32_t ITCR;
    uint32_t ITIP;
    uint32_t ITOP;
    uint32_t TDR;
};

struct gpio_control_block {
    uint32_t GPFSEL0;
    uint32_t GPFSEL1;
    uint8_t pad[144];
    uint32_t GPPUD;
    uint32_t GPPUDCLK0;
};

struct messagebox_control_block {
    uint32_t read;
    uint8_t __pad__[12];
    uint32_t poll;
    uint32_t sender;
    uint32_t status;
    uint32_t config;
    uint32_t write;

    [[nodiscard]] bool status_empty() const volatile {
        return status & (1U << 30U);
    }

    [[nodiscard]] bool status_full() const volatile {
        return status & (1U << 31U);
    }
};

enum class irq_channels : uint64_t {
    system_timer = 1,
    system_timer_3 = 3,
    usb = 9,
    uart = 57,
    emmc = 62 // basic irq 20
};

struct interrupt_controller_control_block {
    uint8_t __pad__[0x200];
    uint32_t irq_basic_pending;
    uint32_t irq_pending_1;
    uint32_t irq_pending_2;

    uint32_t fiq_control;

    uint32_t enable_irq_1;
    uint32_t enable_irq_2;
    uint32_t enable_basic_irq;

    uint32_t disable_irq_1;
    uint32_t disable_irq_2;
    uint32_t disable_basic_irq;
};

struct system_timer_control_block {
    uint32_t control_status;
    uint32_t counter_lo;
    uint32_t counter_hi;
    uint32_t compare0;
    uint32_t compare1;
    uint32_t compare2;
    uint32_t compare3;
};

struct emmc_control_block {
    uint32_t ARG2;
    uint32_t BLKSIZECNT;
    uint32_t ARG1;
    uint32_t CMDTM;
    uint32_t RESP0;
    uint32_t RESP1;
    uint32_t RESP2;
    uint32_t RESP3;
    uint32_t DATA;
    uint32_t STATUS;
    uint32_t CONTROL0;
    uint32_t CONTROL1;
    uint32_t INTERRUPT;
    uint32_t IRPT_MASK;
    uint32_t IRPT_EN;
    uint32_t CONTROL2;
    uint8_t __pad1[0x50-0x40];
    uint32_t FORCE_IRPT;
    uint8_t __pad2[0x70-0x54];
    uint32_t BOOT_TIMEOUT;
    uint32_t DBG_SEL;
    uint8_t __pad3[0x80 - 0x78];
    uint32_t EXRDFIFO_CFG;
    uint32_t EXRDFIFO_EN;
    uint32_t TUNE_STEP;
    uint32_t TUNE_STEPS_STD;
    uint32_t TUNE_STEPS_DDR;
    uint8_t __pad4[0xf0 - 0x94];
    uint32_t SPI_INT_SPT;
    uint32_t SLOTISR_VER;
};

static_assert(offsetof(emmc_control_block, DBG_SEL) == 0x74);
static_assert(offsetof(emmc_control_block, EXRDFIFO_CFG) == 0x80);
static_assert(offsetof(emmc_control_block, SPI_INT_SPT) == 0xf0);

struct sdhost_control_block {
    uint32_t CMD;
    uint32_t ARG;
    uint32_t TOUT;
    uint32_t CDIV;
    const uint32_t RSP0;
    const uint32_t RSP1;
    const uint32_t RSP2;
    const uint32_t RSP3;
    uint32_t HSTS;
    uint32_t VDD;
    uint32_t EDM;
    uint32_t HCFG;
    uint32_t HBCT;
    uint32_t DATA;
    uint32_t HBLC;
};

enum class clocks
{
    reserved,
    emmc,
    uart,
    arm,
    core,
    v3d,
    h264,
    isp,
    sdram,
    pixel,
    pwm,
    hevc,
    emmc2,
    m2mc,
    pixel_bvb
};

enum class clock_tags
{
    get_clock_rate = 0x00030002,
    get_max_clock_rate = 0x00030004,
    set_clock_rate = 0x00038002
};
}