node(
    name="mcu",
    platform="stm32f7",
    exporters=[
        exporter(
            network="udp-internet",
            address="10.0.0.153",
            native=LwipUdpExporter
        ),
    ]
)
