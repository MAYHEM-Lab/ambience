from .defs import *
from queue import Queue
from dijkstar import Graph, find_path


class Networks:
    class Internet:
        UDP = Network("udp-internet", NetworkType.UDP)


def one_hop_import_exports(node: Node):
    return set((imp, exp) for imp in node.importers for exp in imp.assigned_network.exporters if
               exp.assigned_node != node)


def reachability_graph(client: Node):
    graph = {}
    node_queue = Queue()
    node_queue.put(client)

    while not node_queue.empty():
        front = node_queue.get()
        reachables = one_hop_import_exports(front)
        graph[front] = reachables
        for (importer, exporter) in reachables:
            if exporter.assigned_node not in graph:
                node_queue.put(exporter.assigned_node)

    return graph


def all_nodes(graph):
    res = set()
    for node, value in graph.items():
        res.add(node)
        for (imp, exp) in value:
            res.add(exp.assigned_node)
    return res


def import_path(client: Node, server: Node):
    graph = reachability_graph(client)
    nodes = all_nodes(graph)

    if server not in nodes:
        raise RuntimeError("Server is unreachable from client!")

    g = Graph()

    for node, edges in graph.items():
        for (imp, exp) in edges:
            g.add_edge(imp, exp, imp.assigned_network.hop_cost)

    for node in nodes:
        for imp in node.importers:
            g.add_edge(node, imp, imp.hop_cost)
        for exp in node.exporters:
            g.add_edge(exp, node, exp.hop_cost)

    return find_path(g, client, server).nodes

def make_remote_import(client: Node, server: Instance):
    server_node = server.assigned_group.dg.node.node
    path = import_path(client, server_node)
    rpath = path.reverse()

    [source, exporter] = rpath[0:2]
    source_export = exporter.export_service(server)