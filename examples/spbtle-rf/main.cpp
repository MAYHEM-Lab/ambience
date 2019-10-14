//
// Created by fatih on 7/19/19.
//

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <arch/drivers.hpp>
#include <tos/print.hpp>
#include <SPBTLE_RF.h>
#include <tos/uuid.hpp>

void usart_setup(tos::stm32::gpio &g) {
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

static constexpr tos::uuid uuid{
        {0x08, 0x9E, 0x33, 0x3F, 0x66, 0xE8, 0x4D, 0x30, 0xA4, 0x23, 0x96, 0xF1, 0x36, 0xA2, 0x5C, 0x02}
};

extern void attach_HCI_CB(tos::function_ref<void(void *)> callback);

template<class UartT>
void add_gatt_service(UartT &log) {
    uint16_t serv_handle;
    uint16_t char_handle;
    auto ret = aci_gatt_add_serv(UUID_TYPE_128, uuid.id_, PRIMARY_SERVICE, 7,
                                 &serv_handle);

    if (ret != BLE_STATUS_SUCCESS) {
        tos::println(log, "can't register service!");
        tos::this_thread::block_forever();
    }

    ret = aci_gatt_add_char(serv_handle, UUID_TYPE_128,
                            uuid.id_, 1,
                            CHAR_PROP_WRITE_WITHOUT_RESP,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_ATTRIBUTE_WRITE,
                            16, true, &char_handle);

    if (ret != BLE_STATUS_SUCCESS) {
        tos::println(log, "can't register attribute!");
        tos::this_thread::block_forever();
    }

    tos::println(log, "registered", int(serv_handle), int(char_handle));

    auto handler = [&](void *packet) {
        auto hci_pckt = static_cast<hci_uart_pckt *>(packet);
        /* obtain event packet */
        tos::println(log, "got it");

        if (hci_pckt->type != HCI_EVENT_PKT)
            return;

        auto event_pckt = reinterpret_cast<hci_event_pckt *>(&hci_pckt->data[0]);
        tos::println(log, "wow!", int(event_pckt->evt), int(event_pckt->plen));

        switch (event_pckt->evt) {
            case EVT_VENDOR:
                auto blue_evt = reinterpret_cast<evt_blue_aci *>(&event_pckt->data[0]);
                switch (blue_evt->ecode) {
                    case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
                    {
                        auto evt = (evt_gatt_attr_modified_IDB05A1 *) blue_evt->data;
                        tos::println(log, "vendor:",
                                int(evt->attr_handle),
                                int(evt->data_length),
                                int(evt->att_data[0]));

                    }
                    break;
                }
                break;
        }
    };

    attach_HCI_CB(tos::function_ref<void(void *)>(handler));

    tos::println(log, "waiting for events...");
    tos::this_thread::block_forever();
}

void ble_task() {
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

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, usconf);
    tos::println(usart, "hello");
    using namespace std::chrono_literals;
    tos::println(usart, "yoo");

    tos::stm32::exti e;

    tos::stm32::spi s(tos::stm32::detail::spis[2]);
    s.set_8_bit_mode();

    spbtle_rf bl(&s, &e, *erased_alarm, cs_pin, exti_pin, reset);

    tos::println(usart, "began");

    while (true)
    {
        auto build_number = bl.get_fw_id();
        if (!build_number) {
            tos::println(usart, "can't communicate", int(force_error(build_number)));
            continue;
        }
        tos::println(usart, bool(build_number), int(force_get(build_number).build_number));
        break;
    }

    auto gatt = bl.initialize_gatt();
    if (!gatt) {
        tos::println(usart, "gatt failed");
        tos::this_thread::block_forever();
    }

    auto gap = bl.initialize_gap(force_get(gatt), "Tos BLE");
    tos::println(usart, "gap init'd");

    tos::spbtle::advertising adv(1s, "Tos BLE");
    tos::println(usart, "disc started");

    add_gatt_service(usart);

    tos::this_thread::block_forever();
}

static tos::stack_storage<2048> sstore;

void tos_main() {
    tos::launch(sstore, ble_task);
}