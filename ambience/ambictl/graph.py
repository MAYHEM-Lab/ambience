from graphviz import Digraph
from ambictl import defs, sample_group


def to_graph(nodes: [defs.DeployNode], logical: bool = False) -> Digraph:
    g = Digraph(comment="")

    if logical:
        for node in nodes:
            for group in node.groups:
                for serv in group.servs:
                    g.node(name=f"node_s{serv.name}", label=serv.name)
    else:
        for node in nodes:
            with g.subgraph(name=f"cluster_{node.node.name}") as cluster:
                cluster.attr(label=node.node.name)
                for group in node.groups:
                    with cluster.subgraph(name=f"cluster_{group.name}") as gc:
                        gc.attr(label=group.name)
                        for serv in group.servs:
                            gc.node(name=f"node_s{serv.name}", label=serv.name)

    for node in nodes:
        for group in node.groups:
            for serv in group.servs:
                for dep in serv.deps.values():
                    g.edge(f"node_s{serv.name}", f"node_s{dep.name}")

    return g


if __name__ == "__main__":
    nodes = sample_group.sample_deployment()
    print(to_graph(nodes, logical=False))
