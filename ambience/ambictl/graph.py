from graphviz import Digraph
import sys
from ambictl.defs import *
from ambictl.loading import *
from ambictl.ambidecl import *
import mkbuild
import pwd


def to_graph(nodes: [DeployNode], logical: bool = False) -> Digraph:
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
                if hasattr(serv, "deps"):
                    for dep in serv.deps.values():
                        g.edge(f"node_s{serv.name}", f"node_s{dep.name}")

    return g

if __name__ == "__main__":
    dir = sys.argv[1]
    load_dir(dir)
    deployment = ambidecl._finalize()
    print(to_graph(deployment.nodes, logical=False))
    
