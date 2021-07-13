from .defs import *

class ExternService(Instance):
    iface: ServiceInterface
    sync: bool

    def __init__(self, name: str, iface: ServiceInterface, sync: bool):
        super().__init__(name)
        self.iface = iface
        self.extern = True
        self.sync = sync

    def get_interface(self) -> ServiceInterface:
        return self.iface

    def registry_type(self):
        if self.sync:
            return self.iface.sync_server_name()
        return self.iface.async_server_name()

    def __repr__(self):
        return f"ExternService({self.name})"