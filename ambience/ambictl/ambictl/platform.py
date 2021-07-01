import abc
import ambictl.group_loader
import ambictl.deploy_group
import ambictl.node
import ambictl.group


class Platform(abc.ABC):
    loader: GroupLoader

    def __init__(self, loader: GroupLoader):
        self.loader = loader

    @abc.abstractmethod
    def generateBuildDirectories(self, source_dir: str):
        raise NotImplementedError()

    def generateGroupLoader(self, group: DeployGroup):
        return self.loader.generateGroupLoader(group)

    @abc.abstractmethod
    def make_deploy_node(self, node: Node, groups: [Group]):
        raise NotImplementedError()
