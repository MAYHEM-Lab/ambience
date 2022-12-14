from ..defs import *
from ..defs import _write_if_different
from ..group_loader import KernelLoader


class KernelGroup(Group):
    def generate_group_dir(self, build_root):
        group_dir = os.path.join(build_root, self.name)
        os.makedirs(group_dir, exist_ok=True)

        _write_if_different(os.path.join(group_dir, "CMakeLists.txt"), "")

    def generate_loader_dir(self, build_root):
        return KernelLoader().generateGroupLoader(self.dg)
