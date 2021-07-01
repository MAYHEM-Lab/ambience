from __future__ import annotations
from typing import Dict

import os

import ambictl.lidl_module
import ambictl.service

class ServiceInterface:
    module: LidlModule
    full_serv_name: str

    def __init__(self, mod: LidlModule, full_name: str) -> None:
        self.module = mod
        self.full_serv_name = full_name

    def absolute_name(self):
        return self.full_serv_name

    def sync_server_name(self):
        return f"{self.absolute_name()}::sync_server"

    def async_server_name(self):
        return f"{self.absolute_name()}::async_server"

    def get_include(self):
        return os.path.splitext(self.module.file_name())[0] + "_generated.hpp"

    def implement(self, name: str, *, sync: bool, extern: bool,
                  deps: Dict[str, ServiceInterface] = {}, cmake_target: str = "") -> Service:
        return Service(name, cmake_target, self, sync, extern, deps)

