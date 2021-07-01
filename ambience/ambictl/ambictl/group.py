import abc
import ambictl.service_instance
import ambictl.deploy_group
from typing import Set
from toposort import toposort, toposort_flatten


class Group:
    name: str
    servs: Set[ServiceInstance]
    dg: DeployGroup

    def __init__(self, name: str, servs: Set[ServiceInstance]) -> None:
        self.name = name
        self.servs = servs
        for serv in self.servs:
            serv.assigned_group = self

    def servsWithUnmetDeps(self):
        return [serv for serv in self.servs if len(serv.unmetDeps()) != 0]

    def uniqueDeps(self) -> Set[ServiceInstance]:
        return set(dep for serv in self.servs for _, dep in serv.deps.items())

    def uniqueExternalDeps(self):
        return set(dep for dep in self.uniqueDeps() if dep not in self.servs)

    def interfaceDeps(self):
        all_ifaces = {serv.impl.iface: set(serv.impl.deps.values()) for serv in self.servs}
        return toposort_flatten(all_ifaces, sort=False)

    @abc.abstractmethod
    def generate_group_dir(self, build_root):
        raise NotImplementedError()

    @abc.abstractmethod
    def generate_loader_dir(self, build_root):
        raise NotImplementedError()

    @abc.abstractmethod
    def post_generation(self, build_root, conf_dirs):
        pass

    @abc.abstractmethod
    def post_generation2(self, build_root, conf_dirs):
        pass