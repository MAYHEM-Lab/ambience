#!/usr/bin/env python3

import yaml
import sys
import typing
import os
from jinja2 import Template
from typing import NamedTuple

script_dir = os.path.dirname(os.path.realpath(__file__))


class Fragment:
    body: str


class Fragments(NamedTuple):
    header: str
    source: str


class ResourceCompiler:
    def compile(self, node: object):
        raise NotImplementedError()


class StringCompiler(ResourceCompiler):
    header_template: Template
    source_template: Template

    def __init__(self):
        with open(os.path.join(script_dir, "templates/string.hpp.in"), "r") as tmpl:
            self.header_template = Template(tmpl.read())
        with open(os.path.join(script_dir, "templates/string.cpp.jinja"), "r") as tmpl:
            self.source_template = Template(tmpl.read())
        pass

    def compile(self, node):
        header_frag = self.header_template.render({
            "obj_name": "str",
        })

        source_frag = self.source_template.render({
            "obj_name": "str",
            "string_val": node,
            "class_name": "res"
        })

        return Fragments(header_frag, source_frag)


class BlobCompiler(ResourceCompiler):
    header_template: Template
    source_template: Template

    def __init__(self):
        with open(os.path.join(script_dir, "templates/blob.hpp.jinja"), "r") as tmpl:
            self.header_template = Template(tmpl.read())
        with open(os.path.join(script_dir, "templates/blob.cpp.jinja"), "r") as tmpl:
            self.source_template = Template(tmpl.read())
        pass

    def compile(self, node):
        if "from_file" not in node:
            raise ValueError("Blobs must come from a file!")

        initializer = ""

        with open(node["from_file"], "rb") as file:
            initializer = ",".join([str(b) for b in file.read()])

        header_frag = self.header_template.render({
            "obj_name": "blob",
        })

        source_frag = self.source_template.render({
            "obj_name": "blob",
            "initializer": initializer,
            "class_name": "res"
        })

        return Fragments(header_frag, source_frag)


def get_compilers() -> typing.Dict[str, ResourceCompiler]:
    return {
        'string': StringCompiler(),
        'blob': BlobCompiler()
    }


def merge(frags: [Fragments]) -> Fragments:
    return Fragments("\n".join([frag.header for frag in frags]), "\n".join([frag.source for frag in frags]))


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

    def compile(self, file):
        allfrags = []
        for elem in file["resources"]:
            if elem["type"] not in self.compilers:
                print("Unknown resource type: {}".format(elem["type"]))
                continue
            frags = self.compilers[elem["type"]].compile(elem["value"])
            allfrags.append(frags)

        merged = merge(allfrags)
        header = self.header_template.render({
            "declarations": merged.header,
            "namespace": "tos",
            "class_name": "res"
        })
        source = self.source_template.render({
            "definitions": merged.source,
            "namespace": "tos",
            "class_name": "res",
            "header_path": "res_generated.hpp"
        })
        print(header)
        print(source)


if __name__ == "__main__":
    comp = Compiler()
    with open(sys.argv[1], "r") as f:
        file = yaml.safe_load(f)
        comp.compile(file)
