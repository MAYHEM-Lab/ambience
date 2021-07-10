from .defs import *


class ImportedService(Instance):
    _import: Import

    def __init__(self, name: str, imprt: Import):
        super().__init__(name)
        self._import = imprt

    def get_interface(self) -> ServiceInterface:
        return self._import.interface

    def registry_type(self):
        return self._import.interface.sync_server_name()
