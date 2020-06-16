#!/usr/bin/env python3

import yaml
import sys
import typing
import os
from jinja2 import Template
from typing import NamedTuple
import operator, functools

script_dir = os.path.dirname(os.path.realpath(__file__))


class Fragment(NamedTuple):
    # Type of the resource.
    type: str
    # Body of the definition.
    def_body: str
    # Declaration bodies do not contain the declaration of the getter, but rather contain additional declarations
    # if needed.
    extra_decl: str = ""
    # Required non-standard headers
    headers: typing.List[str] = []


class ResourceCompiler:
    def compile(self, node: object) -> Fragment:
        raise NotImplementedError()


class StringCompiler(ResourceCompiler):
    source_template: Template

    def __init__(self):
        with open(os.path.join(script_dir, "templates/string.cpp.jinja"), "r") as tmpl:
            self.source_template = Template(tmpl.read())
        pass

    def compile(self, node) -> Fragment:
        source_frag = self.source_template.render({
            "string_val": node,
        })

        return Fragment("std::string_view", source_frag, headers=["string_view"])


class BlobCompiler(ResourceCompiler):
    source_template: Template

    def __init__(self):
        with open(os.path.join(script_dir, "templates/blob.cpp.jinja"), "r") as tmpl:
            self.source_template = Template(tmpl.read())
        pass

    def compile(self, node) -> Fragment:
        if "from_file" not in node:
            raise ValueError("Blobs must come from a file!")

        with open(node["from_file"], "rb") as file:
            initializer = ",".join([str(b) for b in file.read()])

            source_frag = self.source_template.render({
                "initializer": initializer,
            })

            return Fragment("tos::span<const uint8_t>", source_frag, headers=["tos/span.hpp"])


class KVCompiler(ResourceCompiler):
    def compile(self, node: dict) -> Fragment:
        for key, val in node.items():
            print("{}:{}".format(key, val))
        return super().compile(node)


def get_compilers() -> typing.Dict[str, ResourceCompiler]:
    return {
        'string': StringCompiler(),
        'blob': BlobCompiler(),
        'kv': KVCompiler()
    }


def make_declaration(name: str, frag: Fragment) -> str:
    template = Template("static auto {{name}}() const -> {{type}};")
    return template.render({
        'name': name,
        'type': frag.type
    })


def make_definition(class_name: str, name: str, frag: Fragment) -> str:
    template = Template("""auto {{class_name}}::{{name}}() const -> {{type}} {\n{{body}}\n}""")
    return template.render({
        'class_name': class_name,
        'name': name,
        'type': frag.type,
        'body': frag.def_body
    })


class Compiler:
    header_template: Template
    source_template: Template
    compilers: typing.Dict[str, ResourceCompiler]

    def __init__(self):
        self.compilers = get_compilers()
        with open(os.path.join(script_dir, "templates/resources.hpp.in"), "r") as tmpl:
            self.header_template = Template(tmpl.read())
        with open(os.path.join(script_dir, "templates/resources.cpp.in"), "r") as tmpl:
            self.source_template = Template(tmpl.read())

    def compile(self, resources, namespace, class_name):
        allfrags = []
        for elem in resources:
            if elem["type"] not in self.compilers:
                print("Unknown resource type: {}".format(elem["type"]))
                continue
            frags = self.compilers[elem["type"]].compile(elem["value"])
            allfrags.append((elem["name"], frags))

        header = self.header_template.render({
            "declarations": "\n".join([make_declaration(name, frag) for name, frag in allfrags]),
            "namespace": namespace,
            "class_name": class_name,
            "all_headers": functools.reduce(operator.add, [frag.headers for _, frag in allfrags], [])
        })

        source = self.source_template.render({
            "definitions": "\n".join([make_definition("class_name", name, frag) for name, frag in allfrags]),
            "namespace": namespace,
            "class_name": class_name,
            "header_path": "res_generated.hpp"
        })

        print(header)
        print(source)


if __name__ == "__main__":
    comp = Compiler()
    with open(sys.argv[1], "r") as f:
        file = yaml.safe_load(f)
        comp.compile(file["resources"], namespace="tos", class_name="resources")
