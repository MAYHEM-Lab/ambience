from ambictl import *
from .platforms import *
from .boilerplate import *


def x86_64_pc_node(name: str):
    res = x86_64_pc.make_node(name=name,
                              mems=Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                              exporters=[Networks.Internet.UDP.make_exporter("127.0.0.1", LwipUdpExporter)],
                              importers=[Networks.Internet.UDP.make_importer(LwipUdpImporter)])
    res.node_services.append(ExternService("machine", machine_if, sync=True))
    res.node_services.append(ExternService("fs_block", block_mem_if, sync=True))
    return res


SFO2 = Network("DO-SFO2", NetworkType.UDP)


def digitalocean_vm_node(name: str):
    res = digitalocean_vm.make_node(name=name,
                                    mems=Memories((0x8000000 + 128 * 1024, 256 * 1024),
                                                  (0x20000000 + 64 * 1024, 64 * 1024)),
                                    exporters=[Networks.Internet.UDP.make_exporter("138.68.240.94", LwipUdpExporter),
                                               SFO2.make_exporter("10.138.0.3", LwipUdpExporter)],
                                    importers=[Networks.Internet.UDP.make_importer(LwipUdpImporter),
                                               SFO2.make_importer(LwipUdpImporter)])
    res.node_services.append(ExternService("machine", machine_if, sync=True))
    res.node_services.append(ExternService("fs_block", block_mem_if, sync=True))
    return res


def raspi3(name: str):
    return raspi3.make_node(name=name,
                            mems=Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                            exporters=[LwipUdpExporter()],
                            importers=[LwipUdpImporter()])


home_xbee_net = Network("xbee-home", NetworkType.XBee)


def stm32l475(name: str):
    return stm32.make_node(name=name,
                           mems=Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                           exporters=[],
                           importers=[home_xbee_net.make_importer(lambda: XbeeImporter("tos::stm32::usart*"))])


def hosted_node(name: str):
    return x86_hosted.make_node(name=name,
                                mems=Memories((0x8000000 + 128 * 1024, 256 * 1024),
                                              (0x20000000 + 64 * 1024, 64 * 1024)),
                                exporters=[Networks.Internet.UDP.make_exporter("127.0.0.1", HostedUdpExporter),
                                           home_xbee_net.make_exporter("0x1234",
                                                                       lambda: XbeeExporter(
                                                                           "tos::hosted::usart*"))],
                                importers=[Networks.Internet.UDP.make_importer(HostedUdpImporter)])


nodes = [
    hosted_node("hosted"),
    stm32l475("mcu"),
    digitalocean_vm_node("vm"),
]

for node in nodes:
    node.node_services.append(ExternService("logger", logger_if, sync=True))
    node.node_services.append(ExternService("alarm", alarm_if, sync=False))
    node.node_services.append(ExternService("node_block", block_mem_if, sync=True))
