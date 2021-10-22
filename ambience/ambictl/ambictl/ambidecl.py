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
_exports = {}


def service_iface(serv):
    if "extern" in serv:
        return serv["extern"].get_interface()
    if "serv" in serv:
        return serv["serv"].iface
    if "import" in serv:
        return serv["import"].get_interface()
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


def export(*, service, networks):
    _exports[service] = {
        "service": service,
        "networks": networks
    }


def _finalize_export(export):
    if "done" in export:
        return

    serv = _instance_by_name(export["service"])["instance"]
    nets = {_networks[net]["native"]: conf for net,
            conf in export["networks"].items()}
    node = _nodes[_instance_by_name(export["service"])["node"]]["node"]

    for exporter in node.exporters:
        if exporter.assigned_network in nets:
            serv.export(exporter, config=nets[exporter.assigned_network])

    export["done"] = True


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


def node(*, name, platform, exporters=None, importers=None):
    _nodes[name] = {
        "name": name,
        "platform": platform,
        "importers": importers if importers else [],
        "exporters": exporters if exporters else [],
        "node_services": [],
        "memories": None
    }


def _finalize_node(node):
    if "node" in node:
        return

    platform = _platforms[node["platform"]]
    _finalize_platform(platform)

    node["importers"].extend(platform["importers"])
    node["exporters"].extend(platform["exporters"])
    node["node_services"].extend(copy.deepcopy(platform["node_services"]))

    node["memories"] = platform["memories"]

    importers = [_finalize_importer(importer)
                 for importer in node["importers"]]
    exporters = [_finalize_exporter(exporter)
                 for exporter in node["exporters"]]

    res = platform["native"].make_node(
        name=node["name"],
        mems=node["memories"],
        importers=importers,
        exporters=exporters,
    )

    for serv in node["node_services"]:
        res.node_services.append(serv)

    node["node"] = res


def _finalize_nodes():
    _finalize_platforms()
    for name, node in _nodes.items():
        _finalize_node(node)


def import_service(*, imprt):
    _instances[imprt.name] = {
        "name": imprt.name,
        "import": imprt
    }


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


def _instance_by_name(name):
    return _instances[name]


def _finalize_service(serv):
    if "instance" in serv:
        return serv["instance"]

    print(f"Finalizing service {serv['name']}")

    if "import" in serv:
        serv["instance"] = serv["import"]
        return serv["instance"]

    if "extern" in serv:
        # Extern service, not much to do
        serv["instance"] = serv["extern"]
        return serv["instance"]

    _finalize_node(_nodes[serv["node"]])

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
                             "node" in serv and serv["node"] == node}
            serv["deps"][name] = node_services[name]

    for name, dep in serv["deps"].items():
        if dep.endswith(".localnode"):
            serv["deps"][name] = dep[:-9] + _nodes[serv["node"]]["name"]
            dep = serv["deps"][name]
        if service_iface(_instance_by_name(dep)) != serv["serv"].dependency_type(name):
            raise RuntimeError(
                f"Dependency type mismatch for {name}. Expected {serv['serv'].dependency_type(name).full_serv_name}, got {service_iface(_instance_by_name(dep)).full_serv_name}")

    for name, dep in serv["deps"].items():
        _finalize_service(_instance_by_name(dep))

    for name, dep in serv["deps"].items():
        dep_instance = _instance_by_name(dep)
        if dep_instance["node"] != serv["node"]:
            print("Remote import")
            imports = ambictl.make_remote_import(_nodes[serv["node"]]["node"], _nodes[dep_instance["node"]]["node"],
                                                 dep_instance["instance"])
            for node, svc in imports.items():
                import_service(imprt=svc)
                if not _nodes[node.name]["node"].deploy_node:
                    _nodes[node.name]["node"].node_services.append(svc)
                else:
                    _nodes[node.name]["node"].deploy_node.groups[0].add_service(
                        svc)
                serv["deps"][name] = svc.name

    for name, dep in serv["deps"].items():
        _finalize_service(_instance_by_name(dep))

    deps = {name: _instance_by_name(dep)["instance"]
            for name, dep in serv["deps"].items()}
    for name, dep in deps.items():
        if dep is None:
            raise RuntimeError(f"Unmet dependency {name}")

    serv["instance"] = ambictl.ServiceInstance(
        name=serv["name"],
        impl=serv["serv"],
        deps=deps
    )
    return serv["instance"]


def _finalize_group_step1(group):
    if "done" in group:
        return

    for serv in group["services"]:
        print(f"{serv} to {group['node']}")
        _instance_by_name(serv)["group"] = group
        _instance_by_name(serv)["node"] = group["node"]


def _finalize_group_step2(group, privileged):
    servs = [_finalize_service(_instance_by_name(serv))
             for serv in group["services"]]

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
        _groups[deploy["groups"][0]]["services"].append(
            f"{node_serv.name}.{node['name']}")

    for i, group_name in enumerate(deploy["groups"]):
        if group_name in _instances:
            # Single service group
            group(
                name=f"{group_name}_group",
                services=[
                    group_name
                ]
            )
            deploy["groups"][i] = f"{group_name}_group"
            group_name = f"{group_name}_group"
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

    res = ambictl.Deployment([_finalize_deploy_step2(
        _deploys[deploy]) for deploy in _deploys])

    for export in _exports:
        _finalize_export(_exports[export])

    return res


network(native=ambictl.Networks.Internet.UDP)
