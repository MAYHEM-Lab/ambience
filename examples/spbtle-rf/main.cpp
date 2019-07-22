//
// Created by fatih on 7/19/19.
//

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <arch/drivers.hpp>
#include <tos/print.hpp>
#include <SPBTLE_RF.h>
#include <gp_timer.h>
#include <bluenrg_hal_aci.h>
#include <bluenrg_gap_aci.h>
#include <bluenrg_gatt_aci.h>
#include <bluenrg_utils.h>
#include <bluenrg_gap.h>

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

#if defined(STM32L0)
    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);
#elif defined(STM32L4)
    //auto tx_pin = 22_pin;
    auto rx_pin = 23_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
    gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7);
#elif defined(STM32F1)
    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
#endif
}

void ble_task()
{
    using namespace tos::tos_literals;
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF6, GPIO10 | GPIO11 | GPIO12);

    auto reset = 8_pin;
    auto cs_pin = 61_pin;
    auto exti_pin = 70_pin;

    auto g = tos::open(tos::devs::gpio);

    auto timer = open(tos::devs::timer<2>);
    auto alarm = open(tos::devs::alarm, timer);
    auto erased_alarm = tos::erase_alarm(&alarm);
    alarm_ptr = erased_alarm.get();

    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(tos::usart_parity::disabled)
        .add(tos::usart_stop_bit::one);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, usconf);
    tos::println(usart, "hello");
    using namespace std::chrono_literals;
    alarm_ptr->sleep_for(5ms);
    tos::println(usart, "yoo");

    tos::stm32::spi s(tos::stm32::detail::spis[2]);
    s.set_8_bit_mode();

    spbtle_rf bl(&s, cs_pin, exti_pin, reset);
    bl.begin();

    tos::println(usart, "began");

    uint16_t build_number{};
    auto res = aci_hal_get_fw_build_number(&build_number);
    tos::println(usart, int(res), int(build_number));

    uint8_t hw_id;
    auto stat = getBlueNRGVersion(&hw_id, &build_number);
    tos::println(usart, int(stat), int(hw_id), int(build_number));

    aci_gatt_init();

    tBleStatus ret;
      uint16_t service_handle, dev_name_char_handle, appearance_char_handle;

      ret = aci_gap_init_IDB05A1(1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
      if(ret){
        tos::println(usart, "GAP_Init failed.\n");
          tos::this_thread::block_forever();
      }
      const char *name = "Tos BLE";
      ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t *)name);
      if(ret){
        tos::println(usart, "aci_gatt_update_char_value failed.\n");
      }

#define  ADV_INTERVAL_MIN_MS  800
#define  ADV_INTERVAL_MAX_MS  900
#define  CONN_INTERVAL_MIN_MS 100
#define  CONN_INTERVAL_MAX_MS 300
const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'T','o','s',' ','B','L','E'};
const uint8_t serviceUUIDList[] = {AD_TYPE_16_BIT_SERV_UUID,0x34,0x12};
 ret = aci_gap_set_discoverable(ADV_IND, (ADV_INTERVAL_MIN_MS*1000)/625,
                                       (ADV_INTERVAL_MAX_MS*1000)/625,
                                       STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                       sizeof(local_name), local_name,
                                       0, NULL,
                                       (CONN_INTERVAL_MIN_MS*1000)/1250,
     (CONN_INTERVAL_MAX_MS*1000)/1250);
 tos::println(usart, "disc:", int(ret));

    tos::this_thread::block_forever();
}

static tos::stack_storage<2048> sstore;
void tos_main()
{
    tos::launch(sstore, ble_task);
}
