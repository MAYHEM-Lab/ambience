from ambictl import Networks

node(
    name="sfo2_vm1",
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
    name="sfo2_vm2",
    platform="digitalocean_vm",
    exporters=[
        exporter(
            network="udp-internet",
            address="138.197.207.30",
            native=LwipUdpExporter
        ),
        exporter(
            network="DO-SFO2",
            address="10.138.0.4",
            native=LwipUdpExporter
        ),
    ],
)

# node(
#     name="qemu_vm1",
#     platform="x86_64_pc",
#     exporters=[
#         exporter(
#             network="udp-internet",
#             address="10.0.0.2",
#             native=LwipUdpExporter
#         ),
#     ]
# )

node(
    name="hosted",
    platform="hosted",
    exporters=[
        exporter(
            network="udp-internet",
            address="127.0.0.1",
            native=HostedUdpExporter
        ),
        # exporter(
        #     network="unix-hosted",
        #     address="/",
        #     native=HostedUnixDomainExporter
        # )
        # exporter(
        #     network="xbee-home",
        #     address="0x1234",
        #     native=lambda: XbeeExporter("tos::hosted::usart*")
        # ),
    ]
)

node(
    name="hosted2",
    platform="hosted",
    exporters=[
        exporter(
            network="udp-internet",
            address="127.0.0.1",
            native=HostedUdpExporter
        ),
        # exporter(
        #     network="xbee-home",
        #     address="0x1234",
        #     native=lambda: XbeeExporter("tos::hosted::usart*")
        # ),
    ],
    importers=[
        # importer(
        #     network="unix-hosted",
        #     native=HostedUnixDomainImporter
        # )
    ]
)

node(
    name="hosted3",
    platform="hosted",
    exporters=[
        exporter(
            network="udp-internet",
            address="127.0.0.1",
            native=HostedUdpExporter
        ),
        # exporter(
        #     network="xbee-home",
        #     address="0x1234",
        #     native=lambda: XbeeExporter("tos::hosted::usart*")
        # ),
    ],
    importers=[
        # importer(
        #     network="unix-hosted",
        #     native=HostedUnixDomainImporter
        # )
    ]
)
node(
    name="mcu1",
    platform="stm32l4",
)
