//
// Created by fatih on 3/15/20.
//

#include <arch/drivers.hpp>
#include <iostream>
#include <tos/io/serial_multiplexer.hpp>
#include <tos/print.hpp>
#include <tos/streams.hpp>

namespace asio = boost::asio;

void mux_task() {
    tos::x86::usart port(get_io(), "/dev/ttyACM0", tos::uart::default_115200);
    tos::serial_multiplexer mux(&port, {0, 1, 2, 3, 4});

    for (int i : {0, 1, 2, 3, 4}) {
        tos::launch(tos::alloc_stack, [i, &mux] {
            auto str = *mux.get_stream(i);
            tos::mutex socks_lock;
            std::vector<std::unique_ptr<tos::x86::unix_socket>> socks;

            tos::launch(tos::alloc_stack, [i, &socks, &socks_lock, &str] {
                auto path = "/tmp/tdb/channel" + std::to_string(i);
                ::unlink(path.c_str());
                auto sock = tos::x86::unix_listener(path);
                sock.listen();
                while (true) {
                    std::cerr << "Accepting for " << i << '\n';
                    auto res = sock.accept();
                    std::cerr << "Accepted for " << i << '\n';
                    tos::lock_guard g{socks_lock};
                    socks.emplace_back(std::move(force_get(res)));

                    tos::launch(
                        tos::alloc_stack,
                        [& sock = *socks.back(), &str, i, &socks_lock, &socks] {
                            while (true) {
                                std::array<uint8_t, 1> buffer;
                                auto buf = sock->read(buffer);
                                if (buf.empty()) {
                                    std::cerr << "Socket closed, removing...\n";
                                    tos::lock_guard g{socks_lock};
                                    socks.erase(std::remove_if(
                                        socks.begin(), socks.end(), [&sock](auto& ptr) {
                                            return ptr.get() == &sock;
                                        }));
                                    return;
                                }
                                std::cerr << "Sending " << buf.size() << " bytes to " << i
                                          << '\n';
                                str->write(buf);
                                std::cerr << "Sent " << buf.size() << " bytes to " << i
                                          << '\n';
                            }
                        });
                }
            });

            tos::launch(tos::alloc_stack, [&] {
                while (true) {
                    std::array<uint8_t, 1> buffer;
                    auto buf = str->read(buffer);
                    tos::lock_guard g{socks_lock};
                    for (auto& s : socks) {
                        s->write(buf);
                    }
                }
            });

            tos::this_thread::block_forever();
        });
    }

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, mux_task);
}