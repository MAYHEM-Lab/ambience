from __future__ import annotations
from typing import Dict

import ambictl.service
import ambictl.group


class ServiceInstance:
    name: str
    impl: Service
    deps: Dict[str, ServiceInstance]

    assigned_group: Group

    def unmetDeps(self):
        return [nm for nm, dep in self.deps.items() if dep is None]

    def __init__(self, name: str, impl: Service, deps: Dict[str, ServiceInstance] = {}) -> None:
        self.name = name
        self.impl = impl
        self.deps = deps
        self.assigned_group = None

        for nm, dep in self.impl.deps.items():
            if nm not in self.deps:
                self.deps[nm] = None

    def registry_type(self):
        # if not self.assigned_group.privileged:
        #     return self.impl.iface.async_server_name()

        return self.impl.server_name()
