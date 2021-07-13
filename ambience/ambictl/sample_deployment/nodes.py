from ambictl import Networks

node(
    name="sfo2-vm1",
    platform="digitalocean_vm",
    exporters=[
        exporter(
            network="udp-internet",
            address="138.68.240.94",
            native=LwipUdpExporter
        ),
        exporter(
            network="DO-SFO2",
            address="10.138.0.3",
            native=LwipUdpExporter
        ),
    ],
)

node(
    name="hosted",
    platform="hosted",
    exporters=[
        exporter(
            network="udp-internet",
            address="127.0.0.1",
            native=HostedUdpExporter
        ),
        exporter(
            network="xbee-home",
            address="0x1234",
            native=lambda: XbeeExporter("tos::hosted::usart*")
        ),
    ]
)

node(
    name="mcu1",
    platform="stm32l4",
)
