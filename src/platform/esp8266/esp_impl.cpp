//
// Created by fatih on 4/26/18.
//

extern "C" {
#include "ets_sys.h"
#include "gpio.h"
#include "os_type.h"
#include "osapi.h"

#include <mem.h>
#include <user_interface.h>
#include <xtensa/config/core-isa.h>
}

#include <lwip/timers.h>
#include <tos/compiler.hpp>
#include <tos/debug/debug.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_RF_CAL_ADDR 0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x3fd000
#else
#error "The flash map is not supported"
#endif

void tos_main();
static void entry();

extern "C" {
void ICACHE_FLASH_ATTR user_init() {
    system_init_done_cb(entry);
}

constexpr auto EAGLE_FLASH_BIN_ADDR = static_cast<partition_type_t>(SYSTEM_PARTITION_CUSTOMER_BEGIN + 1);
constexpr auto EAGLE_IROM0TEXT_BIN_ADDR = static_cast<partition_type_t>(SYSTEM_PARTITION_CUSTOMER_BEGIN + 2);

static const partition_item_t at_partition_table[] = {
    {EAGLE_FLASH_BIN_ADDR, 0x00000, 0x10000},
    {EAGLE_IROM0TEXT_BIN_ADDR, 0x10000, 0x60000},
    {SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000}, // 4KB
    {SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000}, // 4KB
    {SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000}, // 12KB
};

void ICACHE_FLASH_ATTR user_pre_init(void) {
    if (!system_partition_table_regist(at_partition_table,
                                       std::size(at_partition_table),
                                       SPI_FLASH_SIZE_MAP)) {
        os_printf("TOS: system_partition_table_regist fail\r\n");
        while (true) {}
    }
    os_printf("TOS: system_partition_table_regist ok\r\n");

    // check if phy binary has been programmed or not
    // It must be programmed at SYSTEM_PARTITION_PHY_DATA_ADDR

    /*auto ptr = reinterpret_cast<const uint8_t*>(SYSTEM_PARTITION_PHY_DATA_ADDR);
    if (*ptr != 0x5) {
        os_printf("bad phy sector\r\n");
        while (true) {}
    }*/
}

void ICACHE_FLASH_ATTR user_rf_pre_init() {}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        break;

    case FLASH_SIZE_64M_MAP_1024_1024:
        rf_cal_sec = 2048 - 5;
        break;
    case FLASH_SIZE_128M_MAP_1024_1024:
        rf_cal_sec = 4096 - 5;
        break;
    default:
        rf_cal_sec = 0;
        break;
    }

    return rf_cal_sec;
}
}

static void ICACHE_FLASH_ATTR main_task(ETSEvent*) {
    // sys_check_timeouts();
    auto res = tos::global::sched.schedule();

    if (res == tos::exit_reason::yield) {
        system_os_post(tos::esp82::main_task_prio, 0, 0);
    }
}

extern "C" {
extern "C" void* __dso_handle;
void* __dso_handle = nullptr;
extern void (*__init_array_start)();
extern void (*__init_array_end)();

static void ICACHE_FLASH_ATTR do_global_ctors() {
    void (**p)();
    for (p = &__init_array_start; p != &__init_array_end; ++p) (*p)();
}

void __register_exitproc() {
}

uint32 ICACHE_FLASH_ATTR espconn_init(uint32) {
    return 1;
}
}

rst_info rst;
static os_event_t arr[16];
static void ICACHE_FLASH_ATTR entry() {
    uart_div_modify(0, UART_CLK_FREQ / (9600));
    struct rst_info* rtc_info_ptr = system_get_rst_info();
    rst = *rtc_info_ptr;

    do_global_ctors();
    tos::kern::enable_interrupts();
    tos_main();
    system_set_os_print(1);
    system_os_task(main_task, tos::esp82::main_task_prio, arr, 16);
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}
