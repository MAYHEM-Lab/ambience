//
// Created by fatih on 10/31/19.
//

#include <arch/drivers.hpp>
#include <ti/drivers/crypto/CryptoCC32XX.h>
#include <tos/ft.hpp>
#include <tos/print.hpp>

std::array<uint8_t, 32> hashv;
void hash(tos::span<const uint8_t> buffer) {
    CryptoCC32XX_init();
    CryptoCC32XX_HmacParams params;
    CryptoCC32XX_HmacParams_init(&params);
    auto cryptoHandle = CryptoCC32XX_open(0, CryptoCC32XX_HMAC);
    if (!cryptoHandle) {
        tos::debug::panic("can't open crypto");
    }

    CryptoCC32XX_sign(cryptoHandle,
                                    CryptoCC32XX_HMAC_SHA256,
                                    const_cast<uint8_t*>(buffer.data()),
                                    buffer.size(),
                                    hashv.data(),
                                    &params);
    CryptoCC32XX_close(cryptoHandle);
}

void task() {
    uint8_t buff[] = {'a', 'b', 'c'};
    hash(buff);

    using namespace tos::tos_literals;
    auto pin = 4_pin;
    tos::cc32xx::gpio g;

    g.set_pin_mode(5_pin, tos::pin_mode::out);
    g.set_pin_mode(6_pin, tos::pin_mode::out);
    g.set_pin_mode(pin, tos::pin_mode::out);
    g.write(pin, tos::digital::high);

    tos::cc32xx::uart uart(0);

    uart.write(hashv);
    tos::println(uart);

    tos::cc32xx::timer tim(0);
    tos::alarm alarm(tim);

    bool val = true;
    while (true) {
        val = !val;
        g.write(pin, val);
        tos::println(uart, "toggle");
        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 1s);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, task);
}
