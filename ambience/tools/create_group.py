def treeify(names: [[str]]):
    level = {}
    for name in names:
        if len(name) == 1:
            level[name[0]] = None
        else:
            if name[0] in level:
                level[name[0]].append(name[1:])
            else:
                level[name[0]] = [name[1:]]
    for key in level.keys():
        if level[key] is None:
            continue
        level[key] = treeify(level[key])
    return level


def visit_elem(key, val):
    if val is None:
        return f"struct {key} {{\nstruct async_server;\ntemplate<class>\nstruct async_zerocopy_client;\n}};\n"

    res = f"namespace {key} {{\n"
    res += traverse_tree_and_emit(val)
    res += "}\n"

    return res


def traverse_tree_and_emit(tree):
    res = ""
    for key, val in tree.items():
        res += visit_elem(key, val)
    return res


def create_fwd_decls(names: [str]):
    parts = [name.split("::") for name in names]
    print(parts)
    print(treeify(parts))
    print(traverse_tree_and_emit(treeify(parts)))


create_fwd_decls(["tos::ae::services::calculator", "tos::services::logger"])
