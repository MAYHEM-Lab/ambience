import os.path

from .ambidecl import *
from . import *
import importlib.util
import importlib
import glob


def load_file(path: str):
    print(f"Loading {path}")
    with open(path, "r") as file:
        contents = file.read()
        exec(contents, globals())


def load_dir(path: str):
    files = glob.glob(os.path.join(path, "*.py"))
    files.sort()
    for file in files:
        load_file(file)
