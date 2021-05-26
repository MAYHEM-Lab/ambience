#!/usr/bin/env python3

import lidlpy;
import argparse

def dump_services(mod: lidlpy.module):
    for serv in mod.services():
        sym = lidlpy.recursive_definition_lookup(mod.get_scope(), serv)
        print(f"\"{'::'.join(lidlpy.absolute_name(sym))}\":")
        for name, proc in serv.all_procedures():
            print(f"  - {name}")

parser = argparse.ArgumentParser(description='Dump services in a lidl schema')
parser.add_argument("-I", action="append", default=[])
parser.add_argument("input", type=str)

if __name__ == "__main__":
    args = parser.parse_args()
    pr = lidlpy.path_resolver()
    for path in args.I:
        pr.add_import_path(path)
    ctx = lidlpy.load_context()
    ctx.set_importer(pr)
    mod = ctx.do_import(args.input, "")
    dump_services(mod)
