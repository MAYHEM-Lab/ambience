from .defs import *
from queue import Queue, PriorityQueue


class Networks:
    class Internet:
        UDP = Network("udp internet", NetworkType.UDP)


def one_hop_importable_nodes(node: Node):
    return set((n, imp.assigned_network) for imp in node.importers for n in imp.assigned_network.exporting_nodes() if n != node)


def reachability_graph(client: Node):
    graph = {}
    node_queue = Queue()
    node_queue.put(client)

    while not node_queue.empty():
        front = node_queue.get()
        reachables = one_hop_importable_nodes(front)
        graph[front] = reachables
        for (reachable, net) in reachables:
            if reachable not in graph:
                node_queue.put(reachable)

    return graph

def all_nodes(graph):
    res = set()
    for node, value in graph.items():
        res.add(node)
        for (node, net) in value:
            res.add(node)
    return res

def import_path(client: Node, server: Node):
    graph = reachability_graph(client)
    nodes = all_nodes(graph)

    if server not in nodes:
        raise RuntimeError("Server is unreachable from client!")

    costs = [10**6] * len(nodes)
    queue = PriorityQueue()
    