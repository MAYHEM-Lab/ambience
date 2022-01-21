from typing import List
from toposort import toposort
import math
import os
import subprocess
from elftools.elf.elffile import ELFFile
import re
from ..defs import *
from ..defs import _write_if_different


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
        # return 1024*1024, 64*1024*1024
        print(f"{binary_path}: {elffile.get_segment(0).header.p_memsz, elffile.get_segment(1).header.p_memsz}")
        return elffile.get_segment(0).header.p_memsz+4096, elffile.get_segment(1).header.p_memsz+4096


def read_entry_point(binary_path: str):
    with open(binary_path, 'rb') as file:
        elffile = ELFFile(file)
        return elffile.header.e_entry


class UserGroup(Group):
    def override_registry_type(self, inst: Instance):
        # Services running in user space always appear as async to the kernel
        return "async"

    def _uniqueDeps(self) -> Set[Instance]:
        return set(dep for serv in self.servs for dep in serv.deps.values())

    def _externalDeps(self):
        return (dep for dep in self._uniqueDeps() if dep not in self.servs)

    def uniqueExternalDeps(self):
        return set(self._externalDeps())

    def servsWithUnmetDeps(self):
        return [serv for serv in self.servs if len(serv.unmetDeps()) != 0]

    def assignNumsToExternalDeps(self):
        if len(self.servsWithUnmetDeps()) != 0:
            raise RuntimeError("Not all dependencies are met!")
        return list((y, x + 1) for x, y in enumerate(self.uniqueExternalDeps()))

    def generateSyncExternalDepsSection(self):
        numbering = self.assignNumsToExternalDeps()
        return "\n".join(
            f"[[maybe_unused]] auto ext_dep{num}_sync = transport.get_sync_service<{serv.get_interface().absolute_name()}, {num}>();" for serv, num in
            numbering)

    def generateExternalDepsSection(self):
        numbering = self.assignNumsToExternalDeps()
        return "\n".join(
            f"[[maybe_unused]] auto ext_dep{num}_async = transport.get_service<{serv.get_interface().absolute_name()}, {num}>();" for serv, num in
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
                args = (serv_name_mapping[dep] + (("_async" if serv.is_async() else "_sync") if dep in ext_deps else "") for dep in serv.deps.values())
                res.append(f"auto {serv.name} = {'co_await ' if serv.is_async() else ''}init_{serv.impl.name}({', '.join(args)});")

        return res

    def generateCmake(self):
        template = env.get_template("user_group/CMakeLists.txt")
        return template.render({
            "group_name": self.name,
            "schemas": self.cmake_targets(),
            "service_targets": (serv.impl.cmake_target for serv in self.servs)
        })

    def generateBody(self):
        template = env.get_template("user_group/group.cpp")
        return template.render({
            "group": self,
            "service_init_sigs": self.generateInitSigSection(),
            "external_deps": self.generateExternalDepsSection() + "\n" + self.generateSyncExternalDepsSection(),
            "service_inits": self._generate_init_section(),
            "group_init": f"::g = new tos::ae::group({', '.join(serv.name for serv in self.servs)});",
            "service_includes": self.cxx_ordered_includes()
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

        src = os.path.join(subdir, "null.cpp")
        _write_if_different(src, "")

        src = os.path.join(subdir, "linker.ld")
        _write_if_different(src, self.generateLinker(
            Memories(rom=(0, 1024*1024), ram=(1024*1024, 1024*1024))))

        src = os.path.join(subdir, "group.cpp")
        _write_if_different(src, self.generateBody())

        src = os.path.join(subdir, "CMakeLists.txt")
        _write_if_different(src, self.generateCmake())

        src = os.path.join(subdir, "interface.cpp")
        _write_if_different(src, self.generateInterface(self.dg.queue_size))

        if self.dg.memories is not None:
            src = os.path.join(subdir, "linker.ld")
            _write_if_different(src, self.generateLinker(self.dg.memories))

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
