//
// Created by fatih on 8/28/19.
//

#include <cstddef>
#include <cstdint>

namespace tos {
template<size_t PortId, size_t PinId>
struct gpio_pin {
    static constexpr auto port_id = PortId;
    static constexpr auto pin_id = PinId;
    static constexpr auto pin_mask = 1U << PinId;
};

struct any_pin {
    uint8_t port_id;
    uint8_t pin_id;

    template<size_t PortId, size_t PinId>
    constexpr any_pin(gpio_pin<PortId, PinId>)
        : port_id{PortId}
        , pin_id{PinId} {
    }
};

template<size_t Id, size_t Sz>
struct gpio_port;

template<size_t Id>
struct gpio_port<Id, 8> {
    static constexpr auto pin0 = gpio_pin<Id, 0>{};
    static constexpr auto pin1 = gpio_pin<Id, 1>{};
    static constexpr auto pin2 = gpio_pin<Id, 2>{};
    static constexpr auto pin3 = gpio_pin<Id, 3>{};
    static constexpr auto pin4 = gpio_pin<Id, 4>{};
    static constexpr auto pin5 = gpio_pin<Id, 5>{};
    static constexpr auto pin6 = gpio_pin<Id, 6>{};
    static constexpr auto pin7 = gpio_pin<Id, 7>{};
};

template<size_t Id>
struct gpio_port<Id, 16> : gpio_port<Id, 8> {
    static constexpr auto pin8 = gpio_pin<Id, 8>{};
    static constexpr auto pin9 = gpio_pin<Id, 9>{};
    static constexpr auto pin10 = gpio_pin<Id, 10>{};
    static constexpr auto pin11 = gpio_pin<Id, 11>{};
    static constexpr auto pin12 = gpio_pin<Id, 12>{};
    static constexpr auto pin13 = gpio_pin<Id, 13>{};
    static constexpr auto pin14 = gpio_pin<Id, 14>{};
    static constexpr auto pin15 = gpio_pin<Id, 15>{};
};

template<size_t Id>
struct gpio_port<Id, 17> : gpio_port<Id, 16> {
    static constexpr auto pin16 = gpio_pin<Id, 16>{};
};

template<size_t Id>
struct gpio_port<Id, 32> : gpio_port<Id, 16> {
    static constexpr auto pin16 = gpio_pin<Id, 16>{};
    static constexpr auto pin17 = gpio_pin<Id, 17>{};
    static constexpr auto pin18 = gpio_pin<Id, 18>{};
    static constexpr auto pin19 = gpio_pin<Id, 19>{};
    static constexpr auto pin20 = gpio_pin<Id, 20>{};
    static constexpr auto pin21 = gpio_pin<Id, 21>{};
    static constexpr auto pin22 = gpio_pin<Id, 22>{};
    static constexpr auto pin23 = gpio_pin<Id, 23>{};
    static constexpr auto pin24 = gpio_pin<Id, 24>{};
    static constexpr auto pin25 = gpio_pin<Id, 25>{};
    static constexpr auto pin26 = gpio_pin<Id, 26>{};
    static constexpr auto pin27 = gpio_pin<Id, 27>{};
    static constexpr auto pin28 = gpio_pin<Id, 28>{};
    static constexpr auto pin29 = gpio_pin<Id, 29>{};
    static constexpr auto pin30 = gpio_pin<Id, 30>{};
    static constexpr auto pin31 = gpio_pin<Id, 31>{};
};
} // namespace tos