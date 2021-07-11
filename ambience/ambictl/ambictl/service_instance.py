from .defs import Service, ServiceInterface, Instance, Group
from typing import Dict


class ServiceInstance(Instance):
    impl: Service
    deps: Dict[str, Instance]

    def unmetDeps(self):
        return [nm for nm, dep in self.deps.items() if dep is None]

    def __init__(self, name: str, impl: Service, deps: Dict[str, Instance] = {}) -> None:
        super().__init__(name)
        self.impl = impl
        self.deps = deps

        for nm, dep in self.impl.deps.items():
            if nm not in self.deps:
                self.deps[nm] = None

    def registry_type(self):
        # if not self.assigned_group.privileged:
        #     return self.impl.iface.async_server_name()

        return self.impl.server_name()

    def get_interface(self) -> ServiceInterface:
        return self.impl.iface

    def get_dependencies(self):
        return self.impl.deps.values()
