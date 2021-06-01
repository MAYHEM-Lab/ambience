import sqlite3

from ambictl import db, defs

mod = defs.LidlModule("foobar.lidl", "bar")

db = db.Database(sqlite3.connect(":memory:"))
db.create_db()
db.register_module(mod)
db.register_interface(mod.get_service("tos::ae::services::filesystem"))
db.get_module("foobar.lidl")
