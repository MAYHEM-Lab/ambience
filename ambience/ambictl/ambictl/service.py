from __future__ import annotations
from typing import Dict

import ambictl.service_instance
import ambictl.service_interface


class Service:
    name: str
    cmake_target: str
    iface: ServiceInterface

    # Whether a service is async/sync is only important if both the client and the server is on the same group.
    # For instance, if a sync service is running in user space but the client is in the kernel, the kernel will still
    # see an async interface.
    sync: bool

    deps: Dict[str, ServiceInterface]

    # An extern service is one that has non-ambience dependencies, and cannot be initialized only with ambience services
    # For instance, a hardware driver service.
    extern: bool

    def __init__(self, name: str, cmake_target: str, iface: ServiceInterface, sync: bool, extern: bool,
                 deps: Dict[str, ServiceInterface] = {}) -> None:
        self.name = name
        self.cmake_target = cmake_target
        self.iface = iface
        self.deps = deps
        self.sync = sync
        self.extern = extern

    def getInitSignature(self) -> str:
        assert not self.extern
        params = (f"{val.sync_server_name() if self.sync else val.async_server_name()}* {key}" for key, val in
                  self.deps.items())
        if self.sync:
            return f"auto init_{self.name}({', '.join(params)}) -> {self.server_name()}*;"
        else:
            return f"auto init_{self.name}({', '.join(params)}) -> tos::Task<{self.server_name()}*>;"

    def instantiate(self, name: str, deps: Dict[str, ServiceInstance] = {}) -> ServiceInstance:
        return ServiceInstance(name, self, deps)

    def server_name(self):
        if self.sync:
            return self.iface.sync_server_name()
        else:
            return self.iface.async_server_name()
