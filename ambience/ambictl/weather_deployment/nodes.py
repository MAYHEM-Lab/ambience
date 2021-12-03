network(native=Network("xbee-home", NetworkType.XBee))

node(
    name="mcu",
    platform="nrf52840",
    exporters=[
        exporter(
            network="xbee-home",
            address="0x1234",
            native=lambda: XbeeExporter("tos::nrf52::uart*")
        ),
    ]
)

node(
    name="vm",
    platform="x86_64_pc",
    exporters=[
        exporter(
            network="udp-internet",
            address="10.0.0.2",
            native=LwipUdpExporter
        ),
    ]
)
