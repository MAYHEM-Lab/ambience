import ambictl.memories
import ambictl.group
import ambictl.deploy_node

class DeployGroup:
    node: DeployNode
    group: Group
    heap_size: int
    queue_size: int
    sizes: (int, int)
    memories: Memories
    entry_point: int

    def __init__(self, node: DeployNode, group: Group, heap_size: int, queue_size: int):
        group.dg = self
        self.node = node
        self.group = group
        self.heap_size = heap_size
        self.queue_size = queue_size
        self.memories = None
        self.sizes = None
        self.entry_point = 0