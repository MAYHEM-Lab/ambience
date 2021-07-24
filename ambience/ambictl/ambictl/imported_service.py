from .defs import *


class ImportedService(Instance):
    _import: Import

    def __init__(self, name: str, imprt: Import):
        super().__init__(name)
        self._import = imprt

    def get_interface(self) -> ServiceInterface:
        return self._import.interface

    def registry_type(self):
        if not self.is_async():
            return self._import.interface.sync_server_name()
        return self._import.interface.async_server_name()

    def is_async(self):
        return self._import.importer.is_async()