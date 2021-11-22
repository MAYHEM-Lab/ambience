import os.path

from .ambidecl import *
from . import *
import importlib.util
import importlib
import glob
import itertools


def load_file(path: str):
    print(f"Loading {path}")
    with open(path, "r") as file:
        contents = file.read()
        exec(contents, globals())


def load_one_dir(path: str):
    files = glob.glob(os.path.join(path, "*.py"))
    files.sort()
    for file in files:
        load_file(file)


def load_dir(path: str):
    path = os.path.realpath(path)

    parts = path.split(os.sep)
    parts = (f"{x}/" for x in parts)
    all_paths = itertools.accumulate(parts)
    for path in all_paths:
        print(path)
        ambience_dir = os.path.join(path, ".ambience")
        if os.path.exists(ambience_dir):
            load_one_dir(ambience_dir)

    load_one_dir(path)