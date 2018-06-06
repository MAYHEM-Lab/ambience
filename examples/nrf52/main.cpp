//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

#include <nrf_delay.h>
#include <nrf_gpio.h>

#include <nrfx_uarte.h>
#include <drivers/include/nrfx_uarte.h>
#include <tos/compiler.hpp>
#include <drivers/common/gpio.hpp>

namespace pin_mode
{
    struct input_t{};
    struct output_t{};
    struct in_pullup_t{};
    struct in_pulldown_t{};

    static constexpr input_t in{};
    static constexpr output_t out{};
    static constexpr in_pullup_t in_pullup{};
    static constexpr in_pulldown_t in_pulldown{};
}

namespace tos
{
    namespace arm
    {
        class gpio
        {
        public:
            static void init() {
                NRF_P0->OUTSET = UINT32_MAX;
            }

            static void set_pin_mode(int pin, pin_mode::input_t)
            {
                NRF_P0->PIN_CNF[pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                                       | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                                       | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                                       | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                                       | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
            }

            static void set_pin_mode(int pin, pin_mode::output_t)
            {
                NRF_P0->PIN_CNF[pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                                      | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                      | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                                      | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                                      | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
            }

            static void write(int pin, tos::true_type) ALWAYS_INLINE
            {
                NRF_P0->OUTSET = (1UL << pin);
            }

            static void write(int pin, tos::false_type) ALWAYS_INLINE
            {
                NRF_P0->OUTCLR = (1UL << pin);
            }

            static void write(int pin, bool val)
            {
                if (!val)
                {
                    return write(pin, tos::false_type{});
                }
                return write(pin, tos::true_type{});
            }
        };
    }

    namespace digital
    {
        static constexpr true_type high{};
        static constexpr false_type low{};
    }

    arm::gpio* open_impl(devs::gpio_t)
    {
        arm::gpio::init();
        return nullptr;
    }
}

tos::semaphore sem{0};

void led1_task()
{
    using namespace tos;
    tos::arm::gpio::write(17, digital::low);
    while (true)
    {
        tos::arm::gpio::write(17, digital::high);
        for (int i = 0; i < 1000; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }

        tos::arm::gpio::write(17, digital::low);
        for (int i = 0; i < 1000; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }
    }
}

void led2_task()
{
    using namespace tos;
    tos::arm::gpio::write(19, digital::low);
    while (true)
    {
        sem.down();
        tos::arm::gpio::write(19, digital::high);

        sem.down();
        tos::arm::gpio::write(19, digital::low);
    }
}

void tos_main()
{
    constexpr nrfx_uarte_t uart { NRF_UARTE0, NRFX_UARTE0_INST_IDX };

    nrfx_uarte_config_t conf{};

    conf.interrupt_priority = 7;
    conf.parity = NRF_UARTE_PARITY_EXCLUDED;
    conf.hwfc = NRF_UARTE_HWFC_DISABLED;
    conf.baudrate = NRF_UARTE_BAUDRATE_115200;
    conf.pselcts = NRF_UARTE_PSEL_DISCONNECTED;
    conf.pselrts = NRF_UARTE_PSEL_DISCONNECTED;
    conf.pselrxd = 8;
    conf.pseltxd = 6;
    conf.p_context = nullptr;

    auto g = tos::open(tos::devs::gpio);

    g->set_pin_mode(17, pin_mode::out);
    g->set_pin_mode(19, pin_mode::out);

    auto res = nrfx_uarte_init(&uart, &conf, [](nrfx_uarte_event_t const *p_event, void *p_context){
        sem.up();
    });

    uint8_t buf[] = { 'h', 'e', 'l', 'l', 'o', '\n' };
    res = nrfx_uarte_tx(&uart, buf, 6);

    tos::launch(led1_task);
    tos::launch(led2_task);
}