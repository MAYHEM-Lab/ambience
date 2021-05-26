from elftools.elf.elffile import ELFFile
from elftools.dwarf.die import DIE
from elftools.dwarf.dwarfinfo import DWARFInfo
from elftools.dwarf.compileunit import CompileUnit
from pprint import PrettyPrinter
import copy


def dump_struct(filename: str, name: str):
    with open(filename, 'rb') as f:
        elffile = ELFFile(f)
        dwarfinfo = elffile.get_dwarf_info()

        for CU in dwarfinfo.iter_CUs():
            for die in CU.iter_DIEs():
                res = check_die(dwarfinfo, CU, die, name)
                if res is not None:
                    return res

        return None


parsed = {}


def read_type_by_offset(dwarf: DWARFInfo, cu: CompileUnit, offset: int):
    if offset not in parsed:
        pointee = dwarf.get_DIE_from_refaddr(offset)
        parsed[offset] = {}
        res = read_type(dwarf, cu, pointee)
        for key in res:
            parsed[offset][key] = res[key]
    return parsed[offset]


def read_type(dwarf: DWARFInfo, cu: CompileUnit, die: DIE):
    if die.tag == "DW_TAG_class_type":
        return read_class(dwarf, cu, die)
    if die.tag == "DW_TAG_structure_type":
        return read_class(dwarf, cu, die)
    if die.tag == "DW_TAG_pointer_type":
        return read_pointer_type(dwarf, cu, die)
    if die.tag == "DW_TAG_typedef":
        return read_typedef(dwarf, cu, die)
    if die.tag == "DW_TAG_base_type":
        return {
            "typename": die.attributes["DW_AT_name"].value.decode("utf8"),
            "byte_size": die.attributes["DW_AT_byte_size"].value
        }
    if die.tag == "DW_TAG_array_type":
        array_of = read_type_by_offset(dwarf, cu, die.attributes["DW_AT_type"].value)
        size = 0
        for child in die.iter_children():
            if child.tag == "DW_TAG_subrange_type":
                size = child.attributes["DW_AT_upper_bound"].value + 1
        res = {
            "array_of": array_of,
            "size": size
        }
        if "DW_AT_name" in die.attributes:
            res["typename"] = die.attributes["DW_AT_name"].value.decode("utf8")
        return res
    return None


def read_typedef(dwarf: DWARFInfo, cu: CompileUnit, die: DIE):
    type = die.attributes["DW_AT_type"].value
    res = {"alias_to": read_type_by_offset(dwarf, cu, type)}
    if "DW_AT_name" in die.attributes:
        res["typename"] = die.attributes["DW_AT_name"].value.decode("utf8")
    return res


def read_pointer_type(dwarf: DWARFInfo, cu: CompileUnit, die: DIE):
    type = die.attributes["DW_AT_type"].value
    pointee = dwarf.get_DIE_from_refaddr(type)
    res = {"pointer_to": read_type_by_offset(dwarf, cu, type)}
    if "DW_AT_name" in pointee.attributes:
        res["typename"] = pointee.attributes["DW_AT_name"].value.decode("utf8")
    return res


def read_class(dwarf: DWARFInfo, cu: CompileUnit, die: DIE):
    res = {"members": {}}
    if "DW_AT_name" in die.attributes:
        res["typename"] = die.attributes["DW_AT_name"].value.decode("utf8")
    for child in die.iter_children():
        if child.tag == "DW_TAG_member":
            name = child.attributes["DW_AT_name"].value.decode("utf8")
            offset = child.attributes["DW_AT_data_member_location"].value
            type = child.attributes["DW_AT_type"].value
            res["members"][name] = {
                "offset": offset,
                "type": read_type_by_offset(dwarf, cu, type),
            }
    if len(res["members"]) == 0:
        del res["members"]
    return res


def check_die(dwarf: DWARFInfo, cu: CompileUnit, die: DIE, searchname: str):
    if die.tag == "DW_TAG_class_type":
        name = die.attributes["DW_AT_name"].value.decode("utf8")
        if name == searchname:
            return read_type(dwarf, cu, die)
    return None

if __name__ == "__main__":
    pp = PrettyPrinter(indent=4)
    scheduler = dump_struct("/home/fatih/tos/cmake-build-cc3220sf/bin/cc3235_demo", "scheduler")
    print(scheduler)
