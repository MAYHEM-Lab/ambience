#include <tos/device/bcm283x_sdhost.hpp>

namespace tos::device {
namespace {
#define HC_CMD_ENABLE       0x8000
#define SDCMD_FAIL_FLAG     0x4000
#define SDCMD_BUSYWAIT      0x800
#define SDCMD_NO_RESPONSE   0x400
#define SDCMD_LONG_RESPONSE 0x200
#define SDCMD_WRITE_CMD     0x80
#define SDCMD_READ_CMD      0x40
#define SDCMD_CMD_MASK      0x3f

#define SDCDIV_MAX_CDIV 0x7ff

#define SDHSTS_BUSY_IRPT    0x400
#define SDHSTS_BLOCK_IRPT   0x200
#define SDHSTS_SDIO_IRPT    0x100
#define SDHSTS_REW_TIME_OUT 0x80
#define SDHSTS_CMD_TIME_OUT 0x40
#define SDHSTS_CRC16_ERROR  0x20
#define SDHSTS_CRC7_ERROR   0x10
#define SDHSTS_FIFO_ERROR   0x08
#define SDHSTS_DATA_FLAG    0x01

#define SDHSTS_CLEAR_MASK                                                            \
    (SDHSTS_BUSY_IRPT | SDHSTS_BLOCK_IRPT | SDHSTS_SDIO_IRPT | SDHSTS_REW_TIME_OUT | \
     SDHSTS_CMD_TIME_OUT | SDHSTS_CRC16_ERROR | SDHSTS_CRC7_ERROR | SDHSTS_FIFO_ERROR)

#define SDHSTS_TRANSFER_ERROR_MASK \
    (SDHSTS_CRC7_ERROR | SDHSTS_CRC16_ERROR | SDHSTS_REW_TIME_OUT | SDHSTS_FIFO_ERROR)

#define SDHSTS_ERROR_MASK (SDHSTS_CMD_TIME_OUT | SDHSTS_TRANSFER_ERROR_MASK)

#define SDHCFG_BUSY_IRPT_EN  BIT(10)
#define SDHCFG_BLOCK_IRPT_EN BIT(8)
#define SDHCFG_SDIO_IRPT_EN  BIT(5)
#define SDHCFG_DATA_IRPT_EN  BIT(4)
#define SDHCFG_SLOW_CARD     BIT(3)
#define SDHCFG_WIDE_EXT_BUS  BIT(2)
#define SDHCFG_WIDE_INT_BUS  BIT(1)
#define SDHCFG_REL_CMD_LINE  BIT(0)

#define SDVDD_POWER_OFF 0
#define SDVDD_POWER_ON  1

#define SDEDM_FORCE_DATA_MODE BIT(19)
#define SDEDM_CLOCK_PULSE     BIT(20)
#define SDEDM_BYPASS          BIT(21)

#define SDEDM_FIFO_FILL_SHIFT 4
#define SDEDM_FIFO_FILL_MASK  0x1f
static uint32_t edm_fifo_fill(uint32_t edm) {
    return (edm >> SDEDM_FIFO_FILL_SHIFT) & SDEDM_FIFO_FILL_MASK;
}

#define SDEDM_WRITE_THRESHOLD_SHIFT 9
#define SDEDM_READ_THRESHOLD_SHIFT  14
#define SDEDM_THRESHOLD_MASK        0x1f

#define SDEDM_FSM_MASK         0xf
#define SDEDM_FSM_IDENTMODE    0x0
#define SDEDM_FSM_DATAMODE     0x1
#define SDEDM_FSM_READDATA     0x2
#define SDEDM_FSM_WRITEDATA    0x3
#define SDEDM_FSM_READWAIT     0x4
#define SDEDM_FSM_READCRC      0x5
#define SDEDM_FSM_WRITECRC     0x6
#define SDEDM_FSM_WRITEWAIT1   0x7
#define SDEDM_FSM_POWERDOWN    0x8
#define SDEDM_FSM_POWERUP      0x9
#define SDEDM_FSM_WRITESTART1  0xa
#define SDEDM_FSM_WRITESTART2  0xb
#define SDEDM_FSM_GENPULSES    0xc
#define SDEDM_FSM_WRITEWAIT2   0xd
#define SDEDM_FSM_STARTPOWDOWN 0xf

#define SDDATA_FIFO_WORDS 16

#define FIFO_READ_THRESHOLD   4
#define FIFO_WRITE_THRESHOLD  4
#define SDDATA_FIFO_PIO_BURST 8

#define SDHST_TIMEOUT_MAX_USEC 1000
} // namespace

auto bcm283x_sdhost::open(volatile bcm283x::sdhost_control_block* sdhost)
    -> expected<std::unique_ptr<bcm283x_sdhost>, errors> {
    sdhost->VDD = SDVDD_POWER_OFF;
    sdhost->CMD = 0;
    sdhost->ARG = 0;

    sdhost->TOUT = 0xF0'00'00;
    sdhost->CDIV = 0;

    sdhost->HSTS = SDHSTS_CLEAR_MASK;
    sdhost->HCFG = 0;
    sdhost->HBCT = 0;
    sdhost->HBLC = 0;

    auto tmp = sdhost->EDM;
    tmp &= ~((SDEDM_THRESHOLD_MASK << SDEDM_READ_THRESHOLD_SHIFT) |
             (SDEDM_THRESHOLD_MASK << SDEDM_WRITE_THRESHOLD_SHIFT));
    tmp |= (FIFO_READ_THRESHOLD << SDEDM_READ_THRESHOLD_SHIFT) |
           (FIFO_WRITE_THRESHOLD << SDEDM_WRITE_THRESHOLD_SHIFT);
    sdhost->EDM = tmp;

    sleep(20ms);
    sdhost->VDD = SDVDD_POWER_ON;

    sleep(40ms);


}
} // namespace tos::device