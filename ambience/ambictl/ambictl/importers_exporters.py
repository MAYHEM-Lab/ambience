from .defs import *

class RestExporter(Exporter):
    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.RestHttp)

    def export_service(self, service, config) -> Export:
        if config is None:
            config = self.get_port_for_service(service)
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"new tos::ae::http_rest_exporter(co_await registry.wait<\"{export.instance.name}\">(), \"{export.config}\");"

    def cxx_includes(self):
        return ["tos/ae/transport/http/host.hpp"]

    def registry_type(self):
        return "tos::ae::http_server"

    def cxx_init_call(self):
        return "tos::ae::create_http_server(80)"

class LwipUdpImporter(Importer):
    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.UDP)

    def make_import(self, service, config) -> Import:
        return Import(self, service, config)

    def import_from(self, export: Export):
        iface = export.instance.get_interface()
        conf = {
            "ip": export.exporter.net_address,
            "port": export.config
        }
        return self.make_import(iface, conf)

    def import_string(self, import_: Import):
        format = "new {}<tos::ae::async_udp_transport>{{tos::udp_endpoint_t{{tos::parse_ipv4_address(\"{}\"), {{{}}} }} }}"
        return format.format(import_.interface.async_stub_client(), import_.config["ip"], import_.config["port"])

    def cxx_includes(self):
        return ["tos/ae/transport/lwip/udp.hpp"]

    def registry_type(self):
        return "tos::ae::lwip_udp_importer"

    def is_async(self):
        return True


class LwipUdpExporter(Exporter):
    allocation: Dict
    next: int

    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.UDP)
        self.allocation = {}
        self.next = 1993

    def get_port_for_service(self, service):
        if service not in self.allocation:
            self.allocation[service] = self.next
            self.next = self.next + 1
        return self.allocation[service]

    def export_service(self, service, config) -> Export:
        if config is None:
            config = self.get_port_for_service(service)
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"new tos::ae::lwip_host(tos::ae::service_host(co_await registry.wait<\"{export.instance.name}\">()), tos::port_num_t{{{export.config}}});"

    def cxx_includes(self):
        return ["tos/ae/transport/lwip/host.hpp"]

    def registry_type(self):
        return "tos::ae::lwip_exporter"

    def cxx_init_call(self):
        return "tos::ae::lwip_exporter{}"


class HostedUdpImporter(Importer):
    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.UDP)

    def make_import(self, service, config) -> Import:
        return Import(self, service, config)

    def import_from(self, export: Export):
        iface = export.instance.get_interface()
        conf = {
            "ip": export.exporter.net_address,
            "port": export.config
        }
        return self.make_import(iface, conf)

    def import_string(self, import_: Import):
        format = "new {}<tos::ae::hosted_udp_transport>{{tos::udp_endpoint_t{{tos::parse_ipv4_address(\"{}\"), {{{}}} }} }}"
        return format.format(import_.interface.sync_stub_client(), import_.config["ip"], import_.config["port"])

    def cxx_includes(self):
        return ["tos/ae/transport/hosted/udp.hpp"]

    def registry_type(self):
        return "tos::ae::hosted_udp_importer"


class HostedUdpExporter(Exporter):
    allocation: Dict
    next: int

    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.UDP)
        self.allocation = {}
        self.next = 1993

    def get_port_for_service(self, service):
        if service not in self.allocation:
            self.allocation[service] = self.next
            self.next = self.next + 1
        return self.allocation[service]

    def export_service(self, service, config) -> Export:
        if config is None:
            config = self.get_port_for_service(service)
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"new tos::ae::hosted_udp_host(tos::ae::service_host(co_await registry.wait<\"{export.instance.name}\">()), tos::port_num_t{{{export.config}}});"

    def cxx_includes(self):
        return ["tos/ae/transport/hosted/udp_host.hpp"]

    def registry_type(self):
        return "tos::ae::hosted_udp_exporter"


class HostedUnixDomainImporter(Importer):
    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.UnixDomain)

    def make_import(self, service, config) -> Import:
        return Import(self, service, config)

    def import_from(self, export: Export):
        iface = export.instance.get_interface()
        conf = export.config
        return self.make_import(iface, conf)

    def import_string(self, import_: Import):
        return f"tos::ae::unix_domain_importer::import_service<{import_.interface.absolute_name()}>(\"{import_.config}\")"

    def cxx_includes(self):
        return ["tos/ae/transport/hosted/unix_domain.hpp"]

    def registry_type(self):
        return "tos::ae::unix_domain_importer"


class HostedUnixDomainExporter(Exporter):
    allocation: Dict
    next: int

    def __init__(self):
        super().__init__(self.__class__.__name__, NetworkType.UnixDomain)
        self.allocation = {}
        self.next = 1

    def get_port_for_service(self, service):
        if service not in self.allocation:
            self.allocation[service] = self.next
            self.next = self.next + 1
        return self.allocation[service]

    def export_service(self, service, config) -> Export:
        if config is None:
            config = f"/tmp/ambience_socket{self.get_port_for_service(service)}"
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"new tos::ae::unix_export(tos::ae::service_host(co_await registry.wait<\"{export.instance.name}\">()), \"{export.config}\");"

    def cxx_includes(self):
        return ["tos/ae/transport/hosted/unix_domain.hpp"]

    def registry_type(self):
        return "tos::ae::unix_domain_exporter"


class XbeeExporter(Exporter):
    allocation: Dict
    next: int
    serial_type: str
    alarm_type: str

    def __init__(self, serial_type, alarm_type="tos::any_alarm*"):
        super().__init__(self.__class__.__name__, NetworkType.XBee)
        self.allocation = {}
        self.next = 0
        self.serial_type = serial_type
        self.alarm_type = alarm_type

    def get_port_for_service(self, service):
        if service not in self.allocation:
            self.allocation[service] = self.next
            self.next = self.next + 1
        return self.allocation[service]

    def export_service(self, service, config) -> Export:
        if config is None:
            config = self.get_port_for_service(service)
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"(co_await registry.wait<\"{self.name}\">())->export_service(tos::ae::service_host(co_await registry.wait<\"{export.instance.name}\">()), tos::ae::xbee_export_args{{.channel = {export.config}}});"

    def cxx_includes(self):
        return ["tos/ae/transport/xbee/xbee_host.hpp", "tos/board.hpp"]

    def registry_type(self):
        return f"tos::ae::xbee_exporter<{self.serial_type}, {self.alarm_type}>"

    def cmake_targets(self):
        return ["ae_xbee_transports"]


class XbeeImporter(Importer):
    serial_type: str
    alarm_type: str

    def __init__(self, serial_type, alarm_type="tos::any_alarm*"):
        super().__init__(self.__class__.__name__, NetworkType.XBee)
        self.serial_type = serial_type
        self.alarm_type = alarm_type

    def make_import(self, service, config) -> Import:
        return Import(self, service, config)

    def import_from(self, export: Export):
        iface = export.instance.get_interface()
        conf = {
            "addr": export.exporter.net_address,
            "channel": export.config
        }
        return self.make_import(iface, conf)

    def import_string(self, import_: Import):
        return f"(co_await registry.wait<\"{self.name}\">())->import_service<{import_.interface.absolute_name()}>(tos::ae::xbee_import_args{{.addr={{{import_.config['addr']}}}, .channel={import_.config['channel']}}})"

    def cxx_includes(self):
        return ["tos/ae/transport/xbee/xbee_transport.hpp", "tos/board.hpp"]

    def registry_type(self):
        return f"tos::ae::xbee_importer<{self.serial_type}, {self.alarm_type}>"

    def cmake_targets(self):
        return ["ae_xbee_transports"]
