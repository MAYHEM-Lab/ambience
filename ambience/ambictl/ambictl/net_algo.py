from .defs import *
from queue import Queue
from dijkstar import Graph, find_path
from .imported_service import ImportedService

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


def make_remote_import(client: Node, server_node: Node, ins: Instance):
    path = import_path(client, server_node)
    print(path)
    path.reverse()
    print(path)

    res = {}
    all_exports = []
    [node, exporter] = path[0:2]
    all_exports.append(ins.export(exporter))
    print(all_exports)

    for [importer, node, exporter] in zip(*(iter(path[2:]),) * 3):
        print((importer, node, exporter))
        imprt = importer.import_from(all_exports[-1])
        serv = ImportedService(f"import_{ins.name}.{node.name}", imprt)
        all_exports.append(serv.export(exporter))
        print(all_exports)
        res[node] = serv


    [importer, node] = path[-2:]
    print((importer, node))
    imprt = importer.import_from(all_exports[-1])
    serv = ImportedService(f"import_{ins.name}.{node.name}", imprt)
    res[node] = serv
    return res

