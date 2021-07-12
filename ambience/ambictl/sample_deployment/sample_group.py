import ambictl
from .node_topology import nodes
from .boilerplate import *

auto = ""
node_local = {}


def pick_node_local(node: ambictl.Node, decl):
    pass


instances = {}
groups = {}
nnodes = {}

group_to_node = {}
service_to_group = {}
service_to_node = {}

def serv_instance(*, name, serv, deps=node_local):
    instances[name] = {
        "name": name,
        "serv": serv,
        "deps": deps
    }


def group(*, name=auto, services):
    for serv in services:
        service_to_group[serv] = name

    groups[name] = {
        "name": name,
        "services": services
    }


def node(*, node: ambictl.Node, groups):
    for group in groups:
        if group in groups:
        group_to_node[group] = node.name
        for serv in group["services"]:
            service_to_node[serv] = node.name

    for serv in node.node_services:
        instances[f"{node.name}.{serv.name}"] = {
            "name": f"{node.name}.{serv.name}"
        }
        service_to_node[f"{node.name}.{serv.name}"] = node.name

    nnodes[node.name] = {
        "node": node,
        "groups": groups
    }


def finalize_service(serv):
    print(f"Finalizing service {serv['name']}")

    if serv["deps"] == node_local:
        # We'll try to satisfy all dependencies with node locals
        deps = {}
        for dep in serv["serv"].deps:
            deps[dep] = node_local
        serv["deps"] = deps

    for name, dep in serv["deps"].items():
        if dep == node_local:
            print(f"Need to satisfy {name} from a node service")
            pass

def finalize_group(group):
    pass

def finalize_node(node):
    all_groups = []
    for g in

def finalize():
    all_nodes = []
    for node in nnodes:
        all_nodes.append(finalize_node(node))
    return all_nodes

serv_instance(
    name="fs",
    serv=littlefs,
    deps={
        "block": "fs_block"
    }
)

serv_instance(
    name="calc",
    serv=basic_calc,
)

serv_instance(
    name="calc2",
    serv=basic_calc
)

serv_instance(
    name="basic_echo",
    serv=basic_echo
)

node(
    node=nodes[0],
    groups=["", "basic_echo"]
)

node(
    node=nodes[2],
    groups=["fs", "calc", "calc2"]
)

nodes(
    node=nodes[1],
    groups=[]
)

finalize()
