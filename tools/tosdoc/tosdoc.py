import glob
import os
import pprint
import tempfile
from compile import compile_highlevel
from misc.ninja_syntax import Writer
import sys
import shutil

from jinja2 import Template

script_dir = os.path.dirname(os.path.realpath(__file__))
dirtemplate = Template(open(os.path.join(script_dir, "templates/index.html.jinja")).read())

pp = pprint.PrettyPrinter(indent=4)


def discover_files(root) -> [str]:
    return glob.glob(os.path.join(root, "**/docs/**/*.adoc"), recursive=True)


def make_compile_command(root):
    compiler_path = os.path.join(script_dir, "compile.py")
    return "python3 '{}' -o $out -R '{}' $in".format(compiler_path, root)


def done(l):
    if len(l) == 0:
        return True
    if len(l[0]) == 0:
        return True
    return not isinstance(l[0][0], str)


def groupby(l):
    out = {}
    for elem in l:
        if elem[0] not in out:
            out[elem[0]] = []
        rest = elem[1:]
        if len(rest) == 0:
            continue
        out[elem[0]].append(rest)
    return out


def make_index(parts):
    if done(parts):
        return parts

    grouped = groupby(parts)

    ret = {}
    for key in grouped:
        ret[key] = make_index(grouped[key])
    return ret


def make_indices(cur_dir, index):
    def is_dir(key):
        return index[key] != []

    generated = []
    elems = []

    for key in index:
        if (is_dir(key)):
            generated += make_indices(os.path.join(cur_dir, key), index[key])
            elems.append(key)
            continue
        elems.append(os.path.splitext(key)[0] + ".html")

    with open(os.path.join(cur_dir, "index.html"), "w") as f:
        f.write(dirtemplate.render({
            'name': cur_dir, #os.path.basename(cur_dir),
            'elems': elems
        }))

    generated.append(os.path.join(cur_dir, "index.html"))

    return generated


if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.realpath(__file__))

    root_dir = os.path.abspath(sys.argv[1])
    build_dir = os.getcwd()

    files = discover_files(root_dir)

    ninja = Writer(open(os.path.join(build_dir, "build.ninja"), "w"))
    ninja.rule(name="tosdocc", command=make_compile_command(root_dir))
    ninja.rule(name="compress", command="tar zcf tosdoc.tar.gz -C {} $in".format(build_dir))

    input_dirs = []
    implicits = []

    for file in files:
        relpath = os.path.relpath(file, root_dir)
        reldir = os.path.dirname(relpath)
        out_dir = os.path.join(build_dir, reldir)
        basename = os.path.basename(file)
        htmlout = os.path.splitext(basename)[0] + ".html"
        outpath = os.path.join(out_dir, htmlout)

        os.makedirs(out_dir, exist_ok=True)

        input_dirs.append(reldir)
        implicits.append(outpath)
        ninja.build(rule="tosdocc", inputs=[file], outputs=[outpath])

    shutil.rmtree(os.path.join(build_dir, "assets"))
    shutil.copytree(os.path.join(script_dir, "assets"), os.path.join(build_dir, "assets"))

    print("Generating index...")
    index = make_index([os.path.relpath(file, root_dir).split(os.path.sep) for file in files])
    generated = make_indices(build_dir, index)
    generated = [os.path.relpath(file, build_dir) for file in generated]

    ninja.build(
        rule="compress",
        inputs=list(set(input_dirs)) + ["index.html", "assets"] + generated,
        outputs=[os.path.join(build_dir, "tosdoc.tar.gz")],
        implicit=implicits)

    print("Build written to {}".format(build_dir))
