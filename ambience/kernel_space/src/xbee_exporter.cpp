#include <registry.hpp>
#include <tos/ae/transport/xbee/xbee_host.hpp>
#include <tos/ae/transport/xbee/xbee_transport.hpp>
#include <tos/board.hpp>

void maybe_init_xbee(tos::any_alarm& alarm) {
    if constexpr (registry.has_service<"XbeeExporter">()) {
        static auto uart = tos::bsp::board_spec::xbee_uart::open();
        auto exporter =
            new tos::ae::xbee_exporter(&uart, &alarm);
        registry.register_service<"XbeeExporter">(exporter);
    }
    if constexpr (registry.has_service<"XbeeImporter">()) {
        static auto uart = tos::bsp::board_spec::xbee_uart::open();
        auto importer = new tos::ae::xbee_importer(&uart, &alarm);
        registry.register_service<"XbeeImporter">(importer);
    }
}