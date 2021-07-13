import ambictl
import copy

auto = ""
node_local = {}


def pick_node_local(node: ambictl.Node, decl):
    pass


_platforms = {}
_nodes = {}
_instances = {}
_groups = {}
_deploys = {}
_networks = {}
_exporters = {}
_importers = {}


def service_iface(serv):
    if "extern" in serv:
        return serv["extern"].iface
    if "serv" in serv:
        return serv["serv"].iface
    print(serv)
    raise RuntimeError("Unknown instance type")


def network(*, native):
    _networks[native.name] = {
        "name": native.name,
        "native": native
    }


def exporter(*, network, address, native):
    return {
        "network": network,
        "address": address,
        "native": native
    }


def _finalize_exporter(exporter):
    net = _networks[exporter["network"]]["native"]
    return net.make_exporter(exporter["address"], exporter["native"])


def importer(*, network, native):
    return {
        "network": network,
        "native": native
    }


def _finalize_importer(importer):
    net = _networks[importer["network"]]["native"]
    return net.make_importer(importer["native"])


def platform(*, name, inherit="", native=None, importers=None, exporters=None, node_services=None, memories=None):
    _platforms[name] = {
        "name": name,
        "inherit": inherit,
        "native": native,
        "importers": importers if importers else [],
        "exporters": exporters if exporters else [],
        "node_services": node_services if node_services else [],
        "memories": memories,
    }


def _finalize_platform(plat):
    if "done" in plat:
        return

    if plat["inherit"] != "":
        inherit = _platforms[plat["inherit"]]
        _finalize_platform(inherit)

        plat["memories"] = inherit["memories"]

        if inherit["native"]:
            assert plat["native"] is None
            plat["native"] = inherit

        plat["importers"].extend(inherit["importers"])
        plat["exporters"].extend(inherit["exporters"])
        plat["node_services"].extend(copy.deepcopy(inherit["node_services"]))

    plat["done"] = True


def _finalize_platforms():
    for name, platform in _platforms.items():
        _finalize_platform(platform)


def node(*, name, platform, exporters=None):
    _nodes[name] = {
        "name": name,
        "platform": platform,
        "importers": [],
        "exporters": exporters if exporters else [],
        "node_services": [],
        "memories": None
    }


def _finalize_node(node):
    if "done" in node:
        return

    platform = _platforms[node["platform"]]
    _finalize_platform(platform)

    node["importers"].extend(platform["importers"])
    node["exporters"].extend(platform["exporters"])
    node["node_services"].extend(copy.deepcopy(platform["node_services"]))

    node["memories"] = platform["memories"]

    importers = [_finalize_importer(importer) for importer in node["importers"]]
    exporters = [_finalize_exporter(exporter) for exporter in node["exporters"]]

    res = platform["native"].make_node(
        name=node["name"],
        mems=node["memories"],
        importers=importers,
        exporters=exporters,
    )

    for serv in node["node_services"]:
        res.node_services.append(serv)

    node["node"] = res
    node["done"] = True


def _finalize_nodes():
    _finalize_platforms()
    for name, node in _nodes.items():
        _finalize_node(node)


def extern_service(*, name, extern):
    _instances[name] = {
        "name": extern.name,
        "extern": extern
    }


def serv_instance(*, name, serv, deps=node_local):
    _instances[name] = {
        "name": name,
        "serv": serv,
        "deps": deps
    }


def group(*, name, services):
    _groups[name] = {
        "name": name,
        "services": services
    }


def deploy(*, node, groups):
    _deploys[node] = {
        "node": node,
        "groups": groups
    }


def _finalize_service(serv):
    if "done" in serv:
        return serv["instance"]

    print(f"Finalizing service {serv['name']}")

    if "extern" in serv:
        # Extern service, not much to do
        serv["done"] = True
        serv["instance"] = serv["extern"]
        return serv["instance"]

    if serv["deps"] == node_local:
        # We'll try to satisfy all dependencies with node locals
        deps = {}
        for dep in serv["serv"].deps:
            deps[dep] = node_local
        serv["deps"] = deps

    for name, dep in serv["deps"].items():
        if dep == node_local:
            print(f"Need to satisfy {name} from a node service")
            node = serv["node"]
            node_services = {serv["name"]: global_name for global_name, serv in _instances.items() if
                             serv["node"] == node}
            serv["deps"][name] = node_services[name]

    for name, dep in serv["deps"].items():
        if dep.endswith(".local"):
            serv["deps"][name] = dep[:-5] + _nodes[serv["node"]]["name"]
            dep = serv["deps"][name]
        if service_iface(_instances[dep]) != serv["serv"].dependency_type(name):
            raise RuntimeError(
                f"Dependency type mismatch for {name}. Expected {serv['serv'].dependency_type(name).full_serv_name}, got {service_iface(_instances[dep]).full_serv_name}")

    for name, dep in serv["deps"].items():
        depins = _instances[dep]
        if depins["node"] != serv["node"]:
            print("Remote import")

    for name, dep in serv["deps"].items():
        _finalize_service(_instances[dep])

    serv["done"] = True
    serv["instance"] = ambictl.ServiceInstance(
        name=serv["name"],
        impl=serv["serv"],
        deps={name: _instances[dep]["instance"] for name, dep in serv["deps"].items()}
    )
    return serv["instance"]


def _finalize_group_step1(group):
    if "done" in group:
        return

    for serv in group["services"]:
        _instances[serv]["group"] = group
        _instances[serv]["node"] = group["node"]


def _finalize_group_step2(group, privileged):
    servs = set(_finalize_service(_instances[serv]) for serv in group["services"])

    res = None
    if privileged:
        res = ambictl.KernelGroup(
            name=group["name"],
            servs=servs
        )
    else:
        res = ambictl.UserGroup(
            name=group["name"],
            servs=servs
        )

    group["done"] = True
    return res


def _finalize_deploy_step1(deploy):
    if "done" in deploy:
        return

    node = _nodes[deploy['node']]
    _finalize_node(node)

    if len(deploy["groups"]) == 0:
        deploy["groups"].append("")

    if deploy["groups"][0] == "":
        group(
            name=f"{node['name']}_priv",
            services=[]
        )
        deploy["groups"][0] = f"{node['name']}_priv"

    if not deploy["groups"][0] in _groups:
        # Single service kernel group
        group(
            name=f"{node['name']}_priv",
            services=[
                deploy["groups"][0]
            ]
        )
        deploy["groups"][0] = f"{node['name']}_priv"

    for node_serv in node["node_services"]:
        extern_service(
            name=f"{node_serv.name}.{node['name']}",
            extern=node_serv
        )
        _groups[deploy["groups"][0]]["services"].append(f"{node_serv.name}.{node['name']}")

    for group_name in deploy["groups"]:
        if group_name in _instances:
            # Single service group
            group(
                name=group_name,
                services=[
                    group_name
                ]
            )
        _groups[group_name]["node"] = deploy["node"]

    for group_name in deploy["groups"]:
        _finalize_group_step1(_groups[group_name])


def _finalize_deploy_step2(deploy):
    groups = []

    groups.append(_finalize_group_step2(_groups[deploy["groups"][0]], True))
    for group_name in deploy["groups"][1:]:
        groups.append(_finalize_group_step2(_groups[group_name], False))

    node = _nodes[deploy['node']]
    return node["node"].deploy(groups)


def _finalize():
    for deploy in _deploys:
        _finalize_deploy_step1(_deploys[deploy])

    return ambictl.Deployment([_finalize_deploy_step2(_deploys[deploy]) for deploy in _deploys])


network(native=ambictl.Networks.Internet.UDP)
