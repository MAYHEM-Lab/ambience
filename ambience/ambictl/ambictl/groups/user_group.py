from typing import List
from toposort import toposort
import math
import os
import subprocess
from elftools.elf.elffile import ELFFile
import re
from ..defs import *


def extract_readelf(build_dir: str) -> str:
    cache_path = os.path.join(build_dir, "CMakeCache.txt")
    with open(cache_path) as cache:
        contents = cache.read()
        pattern = "TOOLCHAIN_READELF:FILEPATH=(.*?)\n"
        res = re.search(pattern, contents)
        return res[1]


def read_seg_sizes(binary_path: str):
    with open(binary_path, 'rb') as file:
        elffile = ELFFile(file)
        return elffile.get_segment(0).header.p_memsz, elffile.get_segment(1).header.p_memsz


def read_entry_point(binary_path: str):
    with open(binary_path, 'rb') as file:
        elffile = ELFFile(file)
        return elffile.header.e_entry


class UserGroup(Group):
    def assignNumsToExternalDeps(self):
        if len(self.servsWithUnmetDeps()) != 0:
            raise RuntimeError("Not all dependencies are met!")
        return list((y, x + 1) for x, y in enumerate(self.uniqueExternalDeps()))

    def generateInitSigSection(self):
        unique_services = set(s.impl for s in self.servs)
        return "\n".join(s.getInitSignature() for s in unique_services)

    def generateExternalDepsSection(self):
        numbering = self.assignNumsToExternalDeps()
        return "\n".join(
            f"auto ext_dep{num} = transport.get_service<{serv.impl.iface.absolute_name()}, {num}>();" for serv, num in
            numbering)

    def _generate_init_section(self):
        ext_deps = self.uniqueExternalDeps()
        serv_name_mapping = {}
        for serv, num in self.assignNumsToExternalDeps():
            serv_name_mapping[serv] = f"ext_dep{num}"

        for serv in self.servs:
            serv_name_mapping[serv] = serv.name

        dep_dict = {serv: set(serv for serv in serv.deps.values() if serv not in ext_deps) for serv in self.servs}

        res = []
        for s in toposort(dep_dict):
            for serv in s:
                args = (serv_name_mapping[dep] for dep in serv.deps.values())
                res.append(f"auto {serv.name} = co_await init_{serv.impl.name}({', '.join(args)});")

        return res

    def generateCmake(self):
        template = env.get_template("user_group/CMakeLists.txt")
        return template.render({
            "group_name": self.name,
            "schemas": (iface.module.cmake_target for iface in self.interfaceDeps()),
            "service_targets": (serv.impl.cmake_target for serv in self.servs)
        })

    def generateBody(self):
        template = env.get_template("user_group/group.cpp")
        return template.render({
            "group": self,
            "service_init_sigs": self.generateInitSigSection(),
            "external_deps": self.generateExternalDepsSection(),
            "service_inits": self._generate_init_section(),
            "group_init": f"::g = new tos::ae::group<{len(self.servs)}>(tos::ae::group<{len(self.servs)}>::make({', '.join(serv.name for serv in self.servs)}));",
            "service_includes": (iface.get_include() for iface in self.interfaceDeps())
        })

    def generateLinker(self, memory: Memories):
        template = env.get_template("user_group/linker.ld")
        return template.render({
            "rom_base": hex(memory.rom[0]),
            "rom_size": f"{math.ceil(memory.rom[1] / 1024)}K",
            "ram_base": hex(memory.ram[0]),
            "ram_size": f"{math.ceil(memory.ram[1] / 1024)}K"
        })

    def generateInterface(self, queue_size):
        template = env.get_template("user_group/interface.cpp")
        return template.render({
            "queue_size": queue_size,
        })

    def generate_group_dir(self, build_root):
        subdir = os.path.join(build_root, self.name)
        os.makedirs(subdir, exist_ok=True)
        with open(os.path.join(subdir, "null.cpp"), mode="w+") as src:
            src.write("")
        with open(os.path.join(subdir, "linker.ld"), mode="w+") as src:
            src.write("")
        with open(os.path.join(subdir, "group.cpp"), mode="w+") as src:
            src.write(self.generateBody())
        with open(os.path.join(subdir, "CMakeLists.txt"), mode="w+") as src:
            src.write(self.generateCmake())
        with open(os.path.join(subdir, "interface.cpp"), mode="w+") as src:
            src.write(self.generateInterface(self.dg.queue_size))
        if self.dg.memories is not None:
            with open(os.path.join(subdir, "linker.ld"), mode="w+") as src:
                src.write(self.generateLinker(self.dg.memories))

    def generate_loader_dir(self, build_root):
        return self.dg.node.node.platform.generateGroupLoader(self.dg)

    def compute_size(self, build_dir: str) -> (int, int):
        args = ["ninja", f"{self.name}_size"]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=build_dir)
        cmake_proc.wait()
        return read_seg_sizes(os.path.join(build_dir, "bin", f"{self.name}_size"))

    def get_entry_point(self, build_dir: str) -> int:
        args = ["ninja", f"{self.name}"]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=build_dir)
        cmake_proc.wait()
        return read_entry_point(os.path.join(build_dir, "bin", f"{self.name}"))

    def post_generation(self, build_dir, conf_dirs):
        self.dg.sizes = self.compute_size(conf_dirs[self.dg.node.node.platform][0])

    def post_generation2(self, build_dir, conf_dirs):
        self.dg.entry_point = self.get_entry_point(conf_dirs[self.dg.node.node.platform][0])
