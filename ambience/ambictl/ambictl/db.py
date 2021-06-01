import sqlite3
from .defs import LidlModule, Group, Platform, BundledElfLoader, Node, Memories, DeployNode, ServiceInterface


class Database:
    conn: sqlite3.Connection

    def __init__(self, conn: sqlite3.Connection):
        self.conn = conn

    def create_db(self):
        cur = self.conn.cursor()
        cur.execute("CREATE TABLE modules (import_str text UNIQUE, cmake_target text);")
        cur.execute(
            """CREATE TABLE ifaces (
                full_iface_name text,
                import_str text,
                FOREIGN KEY(import_str) REFERENCES modules(import_str)
            );""")

    def get_module(self, import_str: str) -> LidlModule:
        cur = self.conn.cursor()

        cur.execute("SELECT * FROM modules WHERE import_str=:import_str LIMIT 1;", {
            "import_str": import_str
        })

        _, cmake_target = cur.fetchone()
        return LidlModule(import_str, cmake_target)

    def load_interface(self, mod: LidlModule, full_name: str) -> LidlModule:
        cur = self.conn.cursor()

        cur.execute("SELECT * FROM ifaces WHERE import_str=:import_str LIMIT 1;", {
            "import_str": import_str
        })

        _, cmake_target = cur.fetchone()
        return LidlModule(import_str, cmake_target)

    def register_module(self, mod: LidlModule):
        cur = self.conn.cursor()
        cur.execute("INSERT INTO modules VALUES (?, ?);", (mod.absolute_import(), mod.cmake_target))
        print(cur.fetchall())

    def register_interface(self, iface: ServiceInterface):
        cur = self.conn.cursor()
        cur.execute("INSERT INTO ifaces VALUES (?, ?);", (iface.full_serv_name, iface.module.absolute_import()))